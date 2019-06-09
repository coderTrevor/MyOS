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

#define MAX_SHARED_HANDLERS 5

// HACKY - Store interrupt handlers in an array for each IRQ which may be shared
// Presently, we know only IRQ's 9 and 11 will wind up shared for the hardware / devices we support
bool (*irq9handlers[MAX_SHARED_HANDLERS])(void) = { 0 };
bool (*irq11handlers[MAX_SHARED_HANDLERS])(void) = { 0 };

int irq9handlerCount = 0;
int irq11handlerCount = 0;

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

void _declspec(naked) irq11_shared_interrupt_handler(void)
{
    _asm pushad;

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("irq 11 shared interrupt handler fired.\n");

    // Call each of the shared int 9 handlers
    for (int i = 0; i < irq11handlerCount; ++i)
    {
        // Call the shared handler. Break from loop if the device owning that handler fired an interrupt.
        if ((*irq11handlers[i])());
            //break;
    }

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + 11);

    _asm
    {
        popad
        iretd
    }
}

void _declspec(naked) irq9_shared_interrupt_handler(void)
{
    _asm pushad;

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("irq 9 shared interrupt handler fired.\n");

    // Call each of the shared int 9 handlers
    for (int i = 0; i < irq9handlerCount; ++i)
    {
        // Call the shared handler. Break from loop if the device owning that handler fired an interrupt.
        if ((*irq9handlers[i])());
            //break;
    }

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + 9);

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

void _declspec(naked) get_graphics_interrupt_handler(int eflags, int cs, bool *graphicsInitialized, int *width, int *height)
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
        terminal_writestring("syscall get graphics interrupt handler fired.\n");

    *graphicsInitialized = !textMode;
    *width = graphicsWidth;
    *height = graphicsHeight;

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

void Interrupts_Add_Shared_Handler(bool (*sharedHandlerAddress)(void), uint8_t irq)
{
    if (irq != 9 && irq != 11)
    {
        kprintf("Only IRQ's 9 and 11 support sharing right now!\n");
        return;
    }

    if (irq == 9)
    {
        irq9handlers[irq9handlerCount++] = sharedHandlerAddress;

        Set_IDT_Entry((unsigned long)irq9_shared_interrupt_handler, HARDWARE_INTERRUPTS_BASE + 9);

        IRQ_Enable_Line(9);
    }
    else
    {
        irq11handlers[irq11handlerCount++] = sharedHandlerAddress;

        Set_IDT_Entry((unsigned long)irq11_shared_interrupt_handler, HARDWARE_INTERRUPTS_BASE + 11);

        IRQ_Enable_Line(11);
    }

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
// because of the EFLAGS and CS registers that the int instruction pushes on the stack (I might have reversed the order of them). 
// Having the eflags and cs as explicit (but unused) parameters in the function definition is, IMHO, an elegant solution to this problem.
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