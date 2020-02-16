#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "Players.h"
#include "InSim.h"

bool strContains(std::string check, std::string condition);
Players getLocalPlayer();
void sendButton(byte clickid, byte t, byte l, byte w, byte h, byte bstyle, char* text);
void requestPackets();
byte multipleOfFour(byte number);