#pragma once
#include "ConnectionManager.h"

#define DEFAULT_PORT "29999" /* TODO: Config file */
#define DEFAULT_OUTGAUGE_PORT "30000"
#define DEFAULT_ADRESS "127.0.0.1"
#define DEFAULT_BUFLEN 512

/* Constructor */
ConnectionManager::ConnectionManager(void* isiPacket)
	: isActive(false),
	_insimSocket(INVALID_SOCKET),
	rBuffer(NULL),
	handlers(NULL),
	OutGaugeHandler(NULL),
	_outgaugeSocket(NULL)
{
	int iResult;

	WSADATA wsaData;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}

	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	iResult = getaddrinfo(DEFAULT_ADRESS, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return;
	}

	ptr = result;

	_insimSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (_insimSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	iResult = connect(_insimSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(_insimSocket);
		_insimSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (_insimSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return;
	}

	/* Send IS_ISI */
	iResult = send(_insimSocket, (char*)isiPacket, sizeof(IS_ISI), 0);

	if (iResult == -1) {
		isActive = false;
		std::cout << "Error while sending IS_ISI, shutting down...\n";
	}

	isActive = true;
	insimThread = std::thread(&ConnectionManager::loopForInsimData, this);
}

/* Receive Loop */
void ConnectionManager::loopForInsimData() { /* Thread Loop Function */
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;

	// Receive data until the server closes the connection
	do {
		iResult = recv(_insimSocket, recvbuf, DEFAULT_BUFLEN, 0);
		if (iResult > 0) { /* There's data */
			// printf("(Main Loop)Bytes received: %d\n", iResult);
			assignBuffer((unsigned char*)recvbuf, iResult);
		}
		else if (iResult == 0) { /* Closing */
			printf("Connection closed\n");
			isActive = false;
		}
		else { /* Error */
			printf("recv failed: %d\n", WSAGetLastError());
			isActive = false;
		}
	} while (iResult > 0);
}

/* Loops through, and dispatches complete packets.
	Else it saves the remaining buffer in a vector
	And appends them in a later iteration / buf receive 
	Event.
*/
void ConnectionManager::assignBuffer(unsigned char* sBuffer, int iResult) { // sBuffer means Socket Buffer
	int size = iResult + rBuffer.size();
	unsigned char* rsBuffer = new unsigned char[size];
	memset(rsBuffer, 0, size);

	if (rBuffer.size() != 0) { // rBuffer means remaining bytes
		for (size_t i = 0; i < rBuffer.size(); i++) {
			rsBuffer[i] = rBuffer[i];
		}
		for (size_t i = rBuffer.size(); i < rBuffer.size() + iResult; i++) {
			rsBuffer[i] = sBuffer[i - rBuffer.size()];
		}

		iResult += (int)rBuffer.size();
		rBuffer.clear();
		sBuffer = rsBuffer;
	}

	if (sBuffer[0] == iResult) {
		dispatchPacket(sBuffer);
		delete[] rsBuffer;
		return;
	}

	while (sBuffer[0] < iResult) {
		dispatchPacket(sBuffer);
		iResult -= sBuffer[0];
		sBuffer += sBuffer[0];
	}
	if (sBuffer[0] > iResult) {
		for (int i = 0; i < iResult; i++) {
			rBuffer.push_back(sBuffer[i]);
		}
	}

	delete[] rsBuffer;
}

/* Binds packets */
// TODO: https://stackoverflow.com/questions/14189440/c-callback-using-class-member/14189561#14189561
void ConnectionManager::BindPacket(void* function, PacketType t) {
	PacketHandler ph;
	memset(&ph, 0, sizeof(PacketHandler));
	ph.Function = function;
	ph.Type = t;

	handlers.push_back(ph);
}

/* Packets dispatcher */
void ConnectionManager::dispatchPacket(unsigned char* recvbuf) {
	for (PacketHandler packethandler : handlers) {
		if (packethandler.Type == recvbuf[1]) {
			((void(*)(void*))packethandler.Function)(recvbuf);
		}
	}
}

// Returns the Number of bytes sent
int ConnectionManager::sendSync(unsigned char* buffer) {
	return send(_insimSocket, (char*)buffer, buffer[0], 0);
}

void ConnectionManager::setupOutgauge(void* outgaugeHandler) {
	/* Code copied from LFSForums, uses deprecated APIs, but works. 
	 * Surprisingly, the new Socket code does not work.
	 *	I'll have to investigate a little, maybe I should be using Local Sockets?
	 */
	OutGaugeHandler = outgaugeHandler;
	// Initialise WinSock version 2.2.
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		WSACleanup();
		std::cerr << "Error: Failed to init WinSock" << std::endl;
	}

	// Create UDP socket.
	_outgaugeSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_outgaugeSocket == INVALID_SOCKET)
	{
		WSACleanup();
		std::cerr << "Error: Could not create socket." << std::endl;
	}

	// Bind to receive UDP packets.
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr(DEFAULT_ADRESS);
	saddr.sin_port = htons(30000);
	if (bind(_outgaugeSocket, (sockaddr*)& saddr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		closesocket(_outgaugeSocket);
		WSACleanup();
		std::cerr << "Error: Could not connect to LFS" << std::endl;
	}

	outgaugeThread = std::thread(&ConnectionManager::loopForOutgaugeData, this);
}

void ConnectionManager::loopForOutgaugeData() {
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;

	// Receive data until the server closes the connection
	do {
		iResult = recv(_outgaugeSocket, recvbuf, DEFAULT_BUFLEN, 0);
		if (iResult > 0) { /* There's data */
			((void(*)(void*))OutGaugeHandler)(recvbuf); /* I need to make this async, socket loop shouldn't be blocked */
		}
		else if (iResult == 0) { /* Closing */
			printf("(Outgauge) Connection closed\n");
		}
		else { /* Error */
			printf("(Outgauge) recv failed: %d\n", WSAGetLastError());
			isActive = false;
		}
	} while (iResult > 0);
}