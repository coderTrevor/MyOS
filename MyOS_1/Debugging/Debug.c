#include "Debug.h"
#include "../printf.h"
#include "../Tasks/Context.h"
#include "../Console_Serial.h"

// This is meant to be used with the debugger support app, custom for MyOS
void DebugStackTrace(unsigned int MaxFrames)
{
    STACK_FRAME *pStackFrame;
    __asm mov pStackFrame, ebp

    serial_printf("Stack trace:\n%s\n", tasks[currentTask].imageName);

    for (unsigned int frame = 0; pStackFrame && frame < MaxFrames; ++frame)
    {
        // Unwind to previous stack frame
        serial_printf("0x%X\n", pStackFrame->eip);
        pStackFrame = pStackFrame->ebp;
    }
}