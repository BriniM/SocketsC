/* InSim Related Includes */
#include "InSim.h"
#include "ConnectionManager.h"
#include "Players.h"
#include "Handlers.h"
#include "Memory.h"

ConnectionManager* _CM;
std::vector<Players> _players;

#define APP_NAME "XPBuddy"

/* Code that gets executed when the program is closing */
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
		case CTRL_CLOSE_EVENT:
			IS_TINY goodbye;
			goodbye.Size = 4;
			goodbye.Type = PacketType::ISP_TINY;
			goodbye.ReqI = 0;
			goodbye.SubT = TinyType::TINY_CLOSE;

			_CM->sendSync((unsigned char*)&goodbye);
			return TRUE;
    }
}

int main() {
	SetConsoleCtrlHandler(CtrlHandler, TRUE);
	IS_ISI isi;
	ZeroMemory(&isi, sizeof(IS_ISI));
	/* Compose ISI */
	isi.Size = sizeof(IS_ISI);
	isi.Type = PacketType::ISP_ISI;
	isi.Flags = ISF_LOCAL | ISF_MCI | ISF_MSO_COLS;
	isi.Interval = 40;
	isi.InSimVer = INSIM_VERSION;
	memcpy(isi.IName, APP_NAME, strlen(APP_NAME));

	/* Pass it to the socket connection manager constructor.*/
	_CM = new ConnectionManager((unsigned char*)&isi);

	_CM->BindPacket(&tinyReceived, PacketType::ISP_TINY);
	_CM->BindPacket(&msoReceived, PacketType::ISP_MSO);
	_CM->BindPacket(&mciReceived, PacketType::ISP_MCI);
	_CM->BindPacket(&ncnReceived, PacketType::ISP_NCN);
	_CM->BindPacket(&cnlReceived, PacketType::ISP_CNL);
	_CM->BindPacket(&nplReceived, PacketType::ISP_NPL);
	_CM->BindPacket(&ismReceived, PacketType::ISP_ISM);
	_CM->BindPacket(&pllReceived, PacketType::ISP_PLL);
	_CM->BindPacket(&plpReceived, PacketType::ISP_PLP);
	_CM->BindPacket(&cprReceived, PacketType::ISP_CPR);

	/* Compose MSL */
	IS_MSL msl;
	ZeroMemory(&msl, sizeof(IS_MSL));
	msl.Size = sizeof(IS_MSL);
	msl.Type = PacketType::ISP_MSL;
	char msg[] = "XPBuddy (C++ Edition) - /o help for commands.";
	memcpy(msl.Msg, msg, sizeof(msg));
	_CM->sendSync((unsigned char*)&msl);

	/* Request NCN, NPL */
	requestPackets();

	/* Setup OutGauge */
	_CM->setupOutgauge(&outgaugeReceived);

	FuelPatch();

	while (_CM->isActive == true) {
		std::this_thread::sleep_for(std::chrono::seconds(20));
	}
}
