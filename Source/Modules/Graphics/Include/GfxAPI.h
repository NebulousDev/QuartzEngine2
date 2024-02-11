#pragma once

#ifdef _WIN32

#ifdef QUARTZ_GRAPHICS_EXPORT
#define QUARTZ_GRAPHICS_API __declspec(dllexport)
#else
#define QUARTZ_GRAPHICS_API __declspec(dllimport)
#endif

#elif defined __GNUC__

#define QUARTZ_GRAPHICS_API __attribute__ ((visibility ("default")))

#else

#define QUARTZ_GRAPHICS_API

#endif