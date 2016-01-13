#pragma once
#include <assert.h>
#ifndef ASSERT
    #ifdef DEBUG
    #define ASSERT(p) assert(p);
    #else
    #define ASSERT(p)
    #endif // DEBUG
#endif


#define  BEGIN_TEST(str)  std::cout<<"BEGIN test"<< #str <<std::endl
#define  END_TEST(str)    std::cout<<"END test" << #str <<std::endl