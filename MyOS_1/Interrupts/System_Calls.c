#include "../misc.h"
#include "Interrupts.h"
//#include "IDT.h"
//#include "../Terminal.h"
#include "System_Calls.h"

// print msg to the string
void SystemCallPrint(char *msg)
{
    const int pointerSize = sizeof(char *);
    __asm
    {
        push msg                // push msg argument to the stack
        int SYSCALL_PRINT       // call print_string_interrupt_handler(msg)
        add esp, pointerSize    // restore value of stack pointer
    }
}