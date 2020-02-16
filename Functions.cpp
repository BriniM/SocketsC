#pragma once
#include "Functions.h"
#include "ConnectionManager.h"

extern std::vector<Players> _players;
extern ConnectionManager* _CM;

byte multipleOfFour(byte number) {
	if (number % 4 == 0) {
		return number;
	}
	return number + (4 - number % 4);
}

bool strContains(std::string check, std::string condition) {
	std::size_t index = check.find(condition);
	if (index != std::string::npos) {
		return true;
	}

	return false;
}

Players getLocalPlayer() {
	Players pl;
	strcpy_s(pl.PName, "Undef");
	for (auto p : _players) {
		if ((p.cFlags & 4) != 4) { // bit 2 is set means the connection is remote.
			return p;
		}
	}
	return pl;
}

void requestPackets() {
	IS_TINY tiny;
	tiny.ReqI = 255;
	tiny.Size = sizeof(IS_TINY);
	tiny.Type = PacketType::ISP_TINY;
	tiny.SubT = TinyType::TINY_NCN;
	_CM->sendSync((unsigned char*)& tiny);

	tiny.SubT = TinyType::TINY_NPL;
	_CM->sendSync((unsigned char*)& tiny);
}

void sendButton(byte clickid, byte t, byte l, byte w, byte h, byte bstyle, char* text) {
	IS_BTN btn;
	ZeroMemory(&btn, multipleOfFour(13 + strlen(text)));

	btn.Size = multipleOfFour(13 + strlen(text));
	btn.Type = PacketType::ISP_BTN;
	btn.ReqI = 1;
	btn.ClickID = clickid;
	btn.BStyle = bstyle;
	btn.T = t;
	btn.L = l;
	btn.W = w;
	btn.H = h;
	strcpy_s(btn.Text, text);

	_CM->sendSync((unsigned char*)&btn);
}