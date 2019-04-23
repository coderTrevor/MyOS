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
void _declspec(naked) print_string_interrupt_handler(char *str)
{
    _asm pushad;

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("syscall interrupt handler fired.\n");

    terminal_writestring(str);

    _asm
    {
        popad
        iretd
    }
}

// TODO: Develop some mechanism to allow printf_ to return an int
// TODO: Investigate why the handler above is able to be implemented so much more simply, while this one needs a bunch of extra assembly to work.
// TODO: This function is always buggy as in interrupt right now, but the behavior changes between release and debug builds
void _declspec(naked) printf_interrupt_handler(const char *fmt, va_list va)
{
    _asm 
    {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi

        pushad
    }

    terminal_print_ulong_hex((uint32_t)fmt);
    terminal_newline();
    terminal_print_ulong_hex((uint32_t)va);
    terminal_newline();

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("printf interrupt handler fired.\n");

    vprintf_(fmt, va);

    //for (;;)
    //    __halt();
    
    _asm
    {
        popad
    
        pop edi
        pop esi
        pop ebx
        pop ebp

        retn
        //iret
    }
}