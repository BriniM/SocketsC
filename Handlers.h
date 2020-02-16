#pragma once
#include <math.h> // Floor
#include "InSim.h"
#include "Players.h"
#include "ConnectionManager.h"
#include "Functions.h"
#include "httprequest.h"

void outgaugeReceived(const OutGaugePack* o);
void msoReceived(const IS_MSO* mso);
void tinyReceived(const IS_TINY* tiny);
void mciReceived(const IS_MCI* mci);
void ncnReceived(const IS_NCN* ncn);
void cnlReceived(const IS_CNL* cnl);
void nplReceived(const IS_NPL* npl);
void ismReceived(const IS_ISM* ism);
void pllReceived(const IS_PLL* pll);
void plpReceived(const IS_PLP* plp);
void cprReceived(const IS_CPR* cpr);
