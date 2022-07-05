
#ifdef _WIN32

#define QUARTZCORE_API __declspec(dllexport)

#elif defined __GNUC__

#define QUARTZCORE_API __attribute__ ((visibility ("default")))

#else

#define QUARTZCORE_API

#endif

/* Exists to trick the compiler into building Core.dll */

int QUARTZCORE_API main() { return 0; }