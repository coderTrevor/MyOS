#include "../misc.h"
#include "Interrupts.h"
//#include "IDT.h"
//#include "../Terminal.h"
#include "System_Calls.h"
#include <stdarg.h>

// print msg to the screen
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

// TODO: return the number of characters printed
int SystemCallPrintf(const char* format, ...)
{
    va_list va;
    va_start(va, format);
    
    const int charPointerSize = sizeof(char *);
    const int va_listSize = sizeof(va_list);

    __asm
    {
        push format             // push format argument to the stack
        push va                 // push va list to the stack
        int SYSCALL_PRINTF      // call print_string_interrupt_handler(msg)
        add esp, charPointerSize + va_listSize    // restore value of stack pointer
    }

    va_end(va);
    return 0;// ret;
}