#include "Handlers.h"

#define ChasingLength 12
#define JoinedLength 13

extern ConnectionManager* _CM;
extern std::vector<Players> _players;

void outgaugeReceived(const OutGaugePack* o) {
	// ClickID, T, L, W, H, BFlags, address
	char fuel[10];
	char speed[6];

	sprintf_s(fuel, "^7%.2f", o->Fuel * 100);
	strcat_s(fuel, "%");
	sendButton(1, 193, 95, 9, 7, ISB_DARK, &fuel[0]);

	sprintf_s(speed, "^7%.0f", o->Speed * 3.6);
	sendButton(2, 193, 106, 9, 7, ISB_DARK, &speed[0]);
}

void msoReceived(const IS_MSO* mso) {
	std::string FullMsg(mso->Msg);
	std::cout << FullMsg << std::endl;

	auto localPlayer = getLocalPlayer();

	if (mso->UserType == MSO_SYSTEM) {
		/* Chase begin event */
		if (strContains(FullMsg, (std::string)"^8 chasing: ")
			|| strContains(FullMsg, (std::string)"^8 joined on "))
		{
			std::cout << "Chase initiated\n";
		}

		/* Chase end events */
	}

	if (mso->UserType == MSO_O) {
		std::transform(FullMsg.begin(),
			FullMsg.end(),
			FullMsg.begin(),
			::tolower);
		if (FullMsg == "help") {
			IS_MSL msl;
			ZeroMemory(&msl, sizeof(IS_MSL));

			msl.Size = sizeof(IS_MSL);
			msl.Type = PacketType::ISP_MSL;
			strcpy_s(msl.Msg, "/o help");
			_CM->sendSync((unsigned char*)&msl);
			strcpy_s(msl.Msg, "/o users");
			_CM->sendSync((unsigned char*)&msl);
		}
		else if (FullMsg == "test") {
			// Work should be done in a separate thread.
			
			std::cout << httprequests::httprequest("www.google.com") << "\n";
		}
		
	}
}

void tinyReceived(const IS_TINY* tiny) {
	std::cout << "Got TINY\n";
	if (tiny->SubT == TinyType::TINY_NONE) {
		_CM->sendSync((unsigned char*)tiny);
	}
}

void ncnReceived(const IS_NCN* ncn) {
	std::cout << "Got NCN\n";
	Players pl;
	memset(&pl, 0, sizeof(Players));
	strcpy_s(pl.UName, ncn->UName);
	strcpy_s(pl.PName, ncn->PName);
	pl.UCID = ncn->UCID;
	pl.cFlags = ncn->Flags;
	_players.push_back(pl);
}

void cnlReceived(const IS_CNL* cnl) {
	for (unsigned char i = 0; i < _players.size(); i++) // No more than 40 connection
	{
		if (_players[i].UCID == cnl->UCID) {
			_players.erase(_players.begin() + i);
		}
	}
}

void mciReceived(const IS_MCI* mci) {
	for (byte i = 0; i < mci->NumC; i++)
	{
		for (auto cnx : _players) {
			if (cnx.PLID == mci->Info[i].PLID) {
				cnx.speed = mci->Info[i].Speed;
				cnx.X = mci->Info[i].X;
				cnx.Y = mci->Info[i].Y;
				cnx.Z = mci->Info[i].Z;
			}
		}
	}
}

void nplReceived(const IS_NPL* npl) {
	std::cout << "Got NPL\n";
	for (auto cnx : _players) {
		if (cnx.UCID == npl->UCID) {
			cnx.onTrack = true;
			cnx.PLID = npl->PLID;
		}
	}
}

void ismReceived(const IS_ISM* ism) {
	_players.clear();
	requestPackets();
}

void pllReceived(const IS_PLL* pll) {
	for (auto cnx : _players) {
		if (cnx.PLID == pll->PLID) {
			cnx.onTrack = false;
			cnx.PLID = 0;
		}
	}
}

void plpReceived(const IS_PLP* plp) {
	for (auto cnx : _players) {
		if (cnx.PLID == plp->PLID) {
			cnx.onTrack = false;
			cnx.PLID = 0;
		}
	}
}

void cprReceived(const IS_CPR* cpr) {
	for (auto cnx : _players) {
		if (cnx.UCID == cpr->UCID) {
			strcpy_s(cnx.PName, cpr->PName);
		}
	}
}