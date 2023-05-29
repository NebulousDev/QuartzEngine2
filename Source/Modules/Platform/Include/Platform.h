#pragma once

#ifdef _WIN32

#ifdef QUARTZ_PLATFORM_EXPORT
#define QUARTZ_PLATFORM_API __declspec(dllexport)
#else
#define QUARTZ_PLATFORM_API __declspec(dllimport)
#endif

#elif defined __GNUC__

#define QUARTZ_PLATFORM_API __attribute__ ((visibility ("default")))

#else

#define QUARTZ_PLATFORM_API

#endif

#include "Types/String.h"
#include "Types/Array.h"
#include "Application.h"

namespace Quartz
{
	struct SystemInfo
	{
		String osName;
		String osBuild;
	};

	struct PlatformSingleton
	{
		SystemInfo		systemInfo;
		Application*	pApplication;
	};

	SystemInfo GenerateSystemInfo();

	bool QUARTZ_PLATFORM_API CreatePlatform(Application* pApplication, PlatformSingleton* pPlatform);
	void QUARTZ_PLATFORM_API DestroyPlatform(PlatformSingleton* pPlatform);
}