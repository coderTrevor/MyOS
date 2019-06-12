#include "../misc.h"
#include "Interrupts.h"
//#include "IDT.h"
#include "../Terminal.h"
#include "System_Calls.h"
#include <stdarg.h>
#include "../myos_io.h"
#include <stdint.h>

int SystemCallFClose(FILE *fp)
{
    int retVal;
    int *pRetVal = &retVal;
    const int pointerSize = sizeof(FILE *) + sizeof(int *);

    __asm
    {
        // push arguments onto stack
        push [pRetVal]
        push [fp]
        int SYSCALL_FCLOSE      // call fclose_interrupt_handler(int fp, int *pRetVal)
        add esp, pointerSize    // restore value of stack pointer
    }

    return retVal;
}

FILE *SystemCallFOpen(const char * filename, const char * mode)
{
    FILE *fp;
    FILE **fpp = &fp;

    const int pointerSize = sizeof(const char *) + sizeof(const char *) + sizeof(FILE *);

    __asm
    {
        // push arguments onto stack
        push [fpp]
        push [mode]
        push [filename]
        int SYSCALL_FOPEN       // call fopen_interrupt_handler(int eflags, int cs, const char *filename, const char *mode, int *fp)
        add esp, pointerSize    // restore value of stack pointer
    }
    return fp;
}

size_t SystemCallFRead(void * bufPtr, size_t elemSize, size_t count, FILE * stream)
{
    size_t retVal;
    size_t *pRetVal = &retVal;
    const int pointerSize = sizeof(void *) + sizeof(size_t) + sizeof(size_t) + sizeof(FILE *) + sizeof(size_t *);

    __asm
    {
        push[pRetVal]          // push arguments onto stack
        push[stream]
        push[count]
        push[elemSize]
        push[bufPtr]
        int SYSCALL_FREAD       // call fread_interrupt_handler(ptr, size, count, stream, pRetVal)
        add esp, pointerSize    // restore value of stack pointer
    }

    return retVal;
}

int SystemCallFSeek(FILE * stream, long int off, int origin)
{
    int retVal;
    int *pRetVal = &retVal;
    const int pointerSize = sizeof(FILE *) + sizeof(long int) + sizeof(int) + sizeof(int *);

    __asm
    {
        push[pRetVal]          // push arguments onto stack
        push[origin]
        push[off]
        push[stream]
        int SYSCALL_FSEEK       // call fseek_interrupt_handler(stream, offset, origin, pRetVal);
        add esp, pointerSize    // restore value of stack pointer
    }

    return retVal;
}

long int SystemCallFTell(FILE * stream)
{
    long int retVal;
    long int *pRetVal = &retVal;
    const int pointerSize = sizeof(FILE *) + sizeof(long int *);

    __asm
    {
        push[pRetVal]           // push arguments onto stack
        push[stream]
        int SYSCALL_FTELL       // call ftell_interrupt_handler(stream, pRetVal);
        add esp, pointerSize    // restore value of stack pointer
    }

    return retVal;
}

// Get graphics info
void SystemCallGetGraphicsInfo(bool *graphicsMode, int *width, int *height)
{
    const int pointerSize = sizeof(bool *) + sizeof(int *) + sizeof(int *);
    __asm
    {
        // push arguments onto stack
        push [height]
        push [width]
        push [graphicsMode]
        int SYSCALL_GET_GRAPHICS_INFO   // call get_graphics_info_interrupt_handler(graphicsArePresent, width, height)
        add esp, pointerSize            // restore value of stack pointer
    }
}

void SystemCallGraphicsBlit(const SDL_Rect *sourceRect, PIXEL_32BIT *image)
{
    const int pointerSize = sizeof(SDL_Rect *) + sizeof(PIXEL_32BIT *);
    printf("gb called\n");
    __asm
    {        
        push[image]                     // push arguments onto stack
        push[sourceRect]
        int SYSCALL_GRAPHICS_BLIT       // call graphics_blit_interrupt_handler(sourceRect, image)
        add esp, pointerSize            // restore value of stack pointer                                        
    }
}

// Allocate memory pages
void SystemCallPageAllocator(unsigned int pages, unsigned int *pPagesAllocated, uint32_t *pReturnVal)
{
    const int pointerSize = sizeof(unsigned int) + sizeof(unsigned int *) + sizeof(uint32_t *);

    __asm
    {
        // push arguments onto stack
        push pReturnVal
        push pPagesAllocated
        push pages
        int  SYSCALL_PAGE_ALLOCATOR  // call PageAllocator(pagesToAllocate, pPagesAllocated, pReturnVal);
        add esp, pointerSize         // restore value of stack pointer
    }
}

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
    va_list va;
    va_start(va, format);

    const int charPointerSize = sizeof(char *);
    const int va_listSize = sizeof(va);

    /*terminal_print_ulong_hex((uint32_t)format);
    terminal_newline();
    terminal_print_ulong_hex((uint32_t)va);
    terminal_newline();*/
    
    // Do printf system call
    __asm
    {
        push [va]               // push va list to the stack
        push format             // push format argument to the stack
        int SYSCALL_PRINTF      // call printf_string_interrupt_handler(format, va)
        add esp, charPointerSize + va_listSize    // restore value of stack pointer
    }

    va_end(va);
    
    return 0;   // TODO: Return number of characters in string
}

void SystemCallTimeDelayMS(uint32_t milliSeconds)
{
    const int pointerSize = sizeof(uint32_t);

    // Do time delay ms system call
    __asm
    {
        push [milliSeconds]         // push milliseconds argument onto the stack
        int SYSCALL_TIME_DELAY_MS   // call time_delay_ms_interrupt_handler(milliSeconds)
        add esp, pointerSize        // restore stack pointer
    }
}