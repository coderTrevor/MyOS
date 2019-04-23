#include <intrin.h>
#include <stdint.h>
#include "../Drivers/Keyboard.h"
#include "PIC.h"
#include "Interrupts.h"
#include "IDT.h"
#include "../System_Specific.h"
#include "../misc.h"
#include "../Console_VGA.h"
#include "../printf.h"
#include "../Terminal.h"

unsigned long interrupts_fired;


void _declspec(naked) default_interrupt_handler(void)
{
    _asm pushad;

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("Default interrupt handler fired.\n");

    _asm
    {
        popad
        iretd
    }
}

void _declspec(naked) default_exception_handler(void)
{
    //_asm pushad;
    //__asm cli;

    ++interrupts_fired;

    terminal_writestring("Default exception handler fired.\n");
    terminal_writestring("System halted.\n");

    for (;;)
        __halt();

    /*_asm
    {
    popad
    iretd
    }*/
}

void _declspec(naked) gpf_exception_handler(void)
{
    //_asm pushad;
    //__asm cli;

    ++interrupts_fired;

    terminal_writestring("General Protection Fault handler fired.\n");
    terminal_writestring("System halted.\n");

    for (;;)
        __halt();

    /*_asm
    {
    popad
    iretd
    }*/
}

void _declspec(naked) page_fault_handler(void)
{
    //_asm pushad;
    //__asm cli;

    ++interrupts_fired;

    terminal_writestring("Page fault handler fired.\n");
    terminal_writestring("System halted.\n");

    for(;;)
        __halt();

    /*_asm
    {
    popad
    iretd
    }*/
}

void _declspec(naked) invalid_opcode_handler(void)
{
    //_asm pushad;
    //__asm cli;

    ++interrupts_fired;

    uint32_t address;

    __asm
    {
        pop [address]
    }
    
    terminal_fill(' ', VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    terminal_writestring("Invalid opcode handler fired.\nEncountered invalid opcode at ");
    terminal_print_ulong_hex(address);
    terminal_writestring(".\nMemory looks like:\n");
    terminal_dumpHex((uint8_t *)address, 256);
    terminal_writestring("System halted.\n");

    for(;;)
        __halt();

    /*_asm
    {
    popad
    iretd
    }*/
}

void Interrupts_Init()
{
    // Initialize the PIC's and remap hardware interrupts 0 - 15 to 32 - 47
    PIC_remap((unsigned char)HARDWARE_INTERRUPTS_BASE, (unsigned char)(HARDWARE_INTERRUPTS_BASE + 8));

    // Start with every interrupt disabled
    uint8_t mask = (uint8_t)~0;
    outb(PIC1_DATA, mask);

    // Enable IRQ 0 for the timer
    IRQ_Enable_Line(0);

    // Enable IRQ 1, the keyboard handler
    IRQ_Enable_Line(1);

    // Initialize the interrupt descriptor table
    IDT_Init();
}

// TODO: using a pointer like this from user space is considered insecure
// NOTE, TODO: See printf_string_interrupt_handler for explanation of eflags and cs parameters
void _declspec(naked) print_string_interrupt_handler(int eflags, int cs, char *str)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;
    _asm
    {
        // prologue
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi

        pushad
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("syscall interrupt handler fired.\n");

    // Write str to the terminal
    terminal_writestring(str);

    _asm
    {
        popad

        // epilogue
        pop edi
        pop esi
        pop ebx
        pop ebp

        iretd
    }
}

// TODO: Develop some mechanism to allow printf_ to return an int
// NOTE: Because this function is called as an interrupt handler, the parameters will not be on the stack where the compiler would expect them,
// because of the EFLAGS and CS registers that the int instruction pushes on the stack. Having the eflags and cs as explicit (but unused)
// parameters in the function definition is, IMHO, an elegant solution to this problem.
// TODO: This will need to be modified when we have a user-mode
void _declspec(naked) printf_interrupt_handler(int eflags, int cs, const char *fmt, va_list va)
{
    // suppress warnings that eflags and cs aren't used
    (void)eflags, (void)cs;

    _asm 
    {
        // prologue
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi

        pushad
    }

    // print the addresses of the fmt and va variables (to see if we know where to find them)
    /*if (debugLevel)
    {
        _asm
        {
            push fmt
            call terminal_print_ulong_hex
            call terminal_newline
            push va
            call terminal_print_ulong_hex
            call terminal_newline
        }
    }*/

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("printf interrupt handler fired.\n");
    
    // Here's where printf actually happens
    vprintf_(fmt, va);
    
    _asm
    {
        popad
    
        // epilogue
        pop edi
        pop esi
        pop ebx
        pop ebp

        iretd
    }
}