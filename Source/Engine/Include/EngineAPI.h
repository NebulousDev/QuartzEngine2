#ifdef _WIN32

#define QUARTZ_ENGINE_API __declspec(dllexport)

#elif defined __GNUC__

#define QUARTZ_ENGINE_API __attribute__ ((visibility ("default")))

#else

#define QUARTZ_ENGINE_API

#endif