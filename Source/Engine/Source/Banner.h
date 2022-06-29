#pragma once

#include "Log.h"

namespace Quartz
{
	inline void PrintBanner()
	{
		LogRaw(LOG_COLOR_CYAN, LOG_COLOR_DEFAULT, L"\n");
		LogRaw(LOG_COLOR_CYAN, LOG_COLOR_DEFAULT, L" ------------------------------------------------------- \n");
		LogRaw(LOG_COLOR_CYAN, LOG_COLOR_DEFAULT, L" |                    Nebulous Games                   | \n");
		LogRaw(LOG_COLOR_CYAN, LOG_COLOR_DEFAULT, L" |                 QUARTZ ENGINE v2.0.0                | \n");
		LogRaw(LOG_COLOR_CYAN, LOG_COLOR_DEFAULT, L" ------------------------------------------------------- \n");
		LogRaw(LOG_COLOR_CYAN, LOG_COLOR_DEFAULT, L"  ~ Copyright © Ben Ratcliff (NebulousDev) 2019-2022 ~   \n\n");
	}
}
