#include <stdint.h>
#include <intrin.h>
#include "Build_Number.h"
#include "Console_VGA.h"
#include "Console_Serial.h"
#include "Console_Shell.h"
#include "GDT.h"
#include "Drivers/Keyboard.h"
#include "System_Specific.h"
#include "paging.h"
#include "multiboot.h"
#include "Drivers/PCI_Bus.h"
#include "Interrupts/PIC.h"
#include "Interrupts/Interrupts.h"
#include "multiboot.h"

int debugLevel = 0;

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. 
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif*/

void KeStartupPhase2(multiboot_info *multibootInfo);

void fill_term(char c, uint8_t foreground_color, uint8_t background_color);

//extern uint32_t paging_space[0x4FFF];
typedef uint32_t ULONG_PTR;

// KeStartup doesn't actually exist in memory where MSVC thinks it does
// so we can't call any functions or use any globals until we setup paging
// (Actually, calling functions does work, but I suspect it may not in the future if the kernel grows too big)
void KeStartup()
{
    uint32_t magic;
    multiboot_info *multibootInfo;

    // Get the magic number and pointer to the multiboot_info structure GRUB passed to the kernel
    __asm
    {
        mov [magic], eax
        mov [multibootInfo], ebx
    }

    // Fill the terminal with one's. If execution stops here, we'll know paging failed.
    fill_term('1', VGA_COLOR_GREEN, VGA_COLOR_BLACK);

    // Enable paging support
    Paging_Enable();

    // Fill terminal with 2's. We'll know paging works
    fill_term('2', VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    // Initialize global descriptor table
    GDT_Init();

    // Initialize terminal interface
    terminal_initialize();
    terminal_writestring("Terminal initialized\n");

    // Now our mapping should be in-line with our project settings, and the following call should work
    KeStartupPhase2(multibootInfo);
}


/**
*   Phase two of kernel startup. Paging is enabled and GDT is valid.
*/
void KeStartupPhase2(multiboot_info *multibootInfo)
{
    // initialize keyboard mapping
    init_key_map();

    // Initialize interrupts
    Interrupts_Init();

    init_serial();

    // Initialize the PCI bus
    PCI_Init();

    // Say Hello
    terminal_writestring("Hello world!\n");
    terminal_writestring("Welcome to My OS (working title) build ");
    terminal_print_int(BUILD_NUMBER);
    terminal_newline();

    // Start Shell
    Shell_Entry(multibootInfo);

    for (;;)
    {
/*#ifdef USE_SERIAL
        char ch = read_serial();
        terminal_putchar(ch);
        //terminal_print_byte((int)ch);
        //terminal_putchar('\n');
        write_serial(ch);
#endif*/
            
        // Check for enter being pressed, so the shell can process commands outside of the keyboard's interrupt handler
        if (shellEnterPressed)
            Shell_Enter_Pressed();

        // Print the number of (non-keyboard) interrupts across the top of the screen
        terminal_writestring_top("Interrupts: ", 61);
        terminal_print_int_top(interrupts_fired, 73);

        // Just do nothing and wait for an interrupt
        __halt();
    }

}


//#pragma section(".text")
//__declspec(allocate(".text"))
#pragma code_seg(".a$0")
__declspec(allocate(".a$0"))
MULTIBOOT_INFO _MultibootInfo = 
{
    (uint32_t)MULTIBOOT_HEADER_MAGIC,
    (uint32_t)MULTIBOOT_HEADER_FLAGS,
    (uint32_t)CHECKSUM,
    (uint32_t)HEADER_ADDRESS,
    (uint32_t)LOADBASE,
    (uint32_t)0, //load end address
    (uint32_t)0, //bss end address
    // This took me forever to figure out! We want a higher-half kernel, so we've told
    // MSVC (via the project settings) to make the base address 0xC000 0000 BUT
    // GRUB doesn't actually copy the kernel there, so we need to do some math
    // to find the actual address of KeStartup.
    (uint32_t)KeStartup - (uint32_t)NonPagingAddressOffset,
    (uint32_t)MODE_TYPE,
    (uint32_t)GRAPHICS_WIDTH,
    (uint32_t)GRAPHICS_HEIGHT,
    (uint32_t)GRAPHICS_DEPTH
};

#pragma comment(linker, "/merge:.text=.a")