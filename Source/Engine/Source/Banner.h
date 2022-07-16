#pragma once

#include "Log.h"

namespace Quartz
{
	inline void PrintBanner()
	{
		LogRaw(LOG_CYAN u8"\n");
		LogRaw(LOG_CYAN u8" ------------------------------------------------------- \n");
		LogRaw(LOG_CYAN u8" |                    Nebulous Games                   | \n");
		LogRaw(LOG_CYAN u8" |                 QUARTZ ENGINE v2.0.0                | \n");
		LogRaw(LOG_CYAN u8" ------------------------------------------------------- \n");
		LogRaw(LOG_CYAN u8"  ~ Copyright \u00A9 Ben Ratcliff (NebulousDev) 2019-2022 ~   \n\n");
	}
}
