#pragma once


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef WIN32
#include "../MyOS_1/Console_VGA.h"
#include "../MyOS_1/Interrupts/System_Calls.h"
//#undef _MSVC_VER
#undef _WIN32
#define HAVE_LIBC 1
#define SDL_ATOMIC_DISABLED 1
#define SDL_EVENTS_DISABLED 1
#define SDL_TIMERS_DISABLED 1
#define HAVE_MALLOC 1
#else
#include <stdio.h>
#endif

class TestClass
{
public:
    void PrintMessage();
};

#ifdef __cplusplus
};
#endif /* __cplusplus */
