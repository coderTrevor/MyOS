#include "../misc.h"
#include "Interrupts.h"
//#include "IDT.h"
#include "../Terminal.h"
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
int __cdecl SystemCallPrintf(const char* format, ...)
{
    //SystemCallPrint("printf called\n"); // TEMPTEMP: Oddly enough, this prints the "format" argument in release mode

    va_list va;
    va_start(va, format);

    const int charPointerSize = sizeof(char *);
    const int va_listSize = sizeof(va);

    /*terminal_print_ulong_hex((uint32_t)format);
    terminal_newline();
    terminal_print_ulong_hex((uint32_t)va);
    terminal_newline();*/
    
    // TEMPTEMP: calling vprintf_ via a system call isn't working yet, but it needs to be:
    __asm
    {
        push[va]               // push va list to the stack
        push format             // push format argument to the stack
        int SYSCALL_PRINTF      // call printf_string_interrupt_handler(msg)
        add esp, charPointerSize + va_listSize    // restore value of stack pointer
    }

    va_end(va);
    
    return 0;   // TODO: Return number of characters in string
}