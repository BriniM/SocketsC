#pragma once
#include <string>

class Players {
    public:
		char UName[24];	// username
		char PName[24];	// nickname
		unsigned char UCID, PLID, cFlags = 0; // cFlags -> Connection Flags
		int X,Y,Z = 0;
		int speed = 0;
		bool onTrack = false;
};