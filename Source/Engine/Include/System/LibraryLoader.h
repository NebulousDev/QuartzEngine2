#pragma once

#include "System/DynamicLibrary.h"

namespace Quartz
{
	DynamicLibrary* LoadDynamicLibrary(const char* pathName);

	bool FreeDynamicLibrary(DynamicLibrary* pDynamicLibrary);
}