#pragma once

#include "Types/String.h"

namespace Quartz
{
	struct SystemInfo
	{
		String osName;
		String osBuild;
	};

	SystemInfo GenerateSystemInfo();
}