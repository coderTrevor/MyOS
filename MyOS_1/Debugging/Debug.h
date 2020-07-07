#pragma once

#include <stdint.h>

typedef struct STACK_FRAME 
{
    struct STACK_FRAME* ebp;
    uint32_t eip;
}STACK_FRAME;

void DebugStackTrace(unsigned int MaxFrames);