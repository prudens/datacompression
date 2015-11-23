#pragma once
#include <assert.h>
#ifndef ASSERT
    #ifdef DEBUG
    #define ASSERT(p) assert(p);
    #else
    #define ASSERT(p)
    #endif // DEBUG
#endif
