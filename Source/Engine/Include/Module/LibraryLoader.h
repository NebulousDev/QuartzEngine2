#pragma once

#include "Module/DynamicLibrary.h"

namespace Quartz
{
	DynamicLibrary* LoadDynamicLibrary(const char* pathName);

	bool FreeDynamicLibrary(DynamicLibrary* pDynamicLibrary);
}