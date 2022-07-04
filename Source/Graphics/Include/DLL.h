#ifdef _WIN32

#define QUARTZENGINE_API __declspec(dllexport)

#elif defined __GNUC__

#define QUARTZENGINE_API __attribute__ ((visibility ("default")))

#else

#define QUARTZENGINE_API

#endif
