#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <iostream> // printf, cout
#include <vector>
#include <thread> // Threads
#include <winsock2.h> // Socket
#include <ws2tcpip.h> // Socket
#include <iphlpapi.h> // Socket

#include "InSim.h" // InSim Packets Definition

class ConnectionManager
{
public:
	ConnectionManager(void* isiPacket); /* Class Constructor*/
	void setupOutgauge(void* outgaugeHandler);
	SOCKET _insimSocket; /* Socket Object */
	SOCKET _outgaugeSocket;
	int sendSync(unsigned char* buffer);
	bool isActive; /* Connected or not */
	void BindPacket(void* function, PacketType t);

private:
	std::vector<char> rBuffer;
	std::vector<PacketHandler> handlers;
	std::thread insimThread;
	std::thread outgaugeThread;
	void* OutGaugeHandler;
	void loopForOutgaugeData();
	void loopForInsimData();
	void assignBuffer(unsigned char* sBuffer, int iResult);
	void dispatchPacket(unsigned char* recvbuf);
};