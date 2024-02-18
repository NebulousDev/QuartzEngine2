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