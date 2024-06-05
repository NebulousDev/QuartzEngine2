#pragma once

#include <assert.h>

#if ~NDEBUG

#ifndef DEBUG_ONLY
#define DEBUG_ONLY if(true)
#endif

#ifndef DEBUG_ASSERT
#define DEBUG_ASSERT(x) assert(x)
#endif

#else

#ifndef DEBUG_ONLY
#define DEBUG_ONLY if(false)
#endif

#ifndef DEBUG_ASSERT
#define DEBUG_ASSERT(x)
#endif

#endif