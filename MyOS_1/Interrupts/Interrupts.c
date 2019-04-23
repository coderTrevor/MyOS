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
// TODO: This isn't actually right: I think we end up printing the msg parameter that is on the stack from the SystemCallPrint() function.
// In fact, this is probably only working due to "luck." See printf_interrupt_handler below for more details.
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
// NOTE: Because this function is called as an interrupt handler, the parameters will not be on the stack where the compiler will expect them,
// because of the EFLAGS register that the int instruction pushes on the stack (I think).
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

    // print the addresses of the fmt and va variables (to see if we know where to find them)
    if (debugLevel)
    {
        _asm
        {
            push fmt + 8
            call terminal_print_ulong_hex
            call terminal_newline
            push va + 8
            call terminal_print_ulong_hex
            call terminal_newline
        }
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("printf interrupt handler fired.\n");
    
    _asm
    {
        push va + 8
        push fmt + 8
        call vprintf_   // call vprintf_(fmt, va), correcting for the stack discrepency
    }
    
    _asm
    {
        popad
    
        pop edi
        pop esi
        pop ebx
        pop ebp

        iretd
    }
}