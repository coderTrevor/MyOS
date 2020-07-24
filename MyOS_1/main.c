#include <stdint.h>
#include "Build_Number.h"
#include "Console_VGA.h"
#include "Console_Serial.h"
#include "Console_Shell.h"
#include "Tasks/Context.h"
#include "GDT.h"
#include "Drivers/Keyboard.h"
#include "System_Specific.h"
#include "paging.h"
#include "multiboot.h"
#include "Drivers/PCI_Bus.h"
#include "Interrupts/PIC.h"
#include "Interrupts/Interrupts.h"
#include "multiboot.h"
#include "Terminal.h"
#include "Graphics/Display_HAL.h"
#include "Timers/System_Clock.h"
#include "Drivers/Sound_Blaster_16.h"
#include "Networking/TFTP.h"
#include "Executables/Batch_Files.h"
#include "Networking/IPv4.h"
#include "Interrupts/System_Calls.h"
#include "printf.h"
#include "Drivers/PS2_Mouse.h"
#include "Graphics/Cursor.h"

int debugLevel = 0;
bool showOverlay = true;
bool cursorEnabled = true;  // Add ability for GUI shell to handle the cursor directly in the future

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. 
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif*/

void Autoexec(void);

void KeStartupPhase2(multiboot_info *multibootInfo);

void fill_term(char c, uint8_t foreground_color, uint8_t background_color);

//extern uint32_t paging_space[0x4FFF];
typedef uint32_t ULONG_PTR;

char timeString[9];

// Jump to the memory location the kernel is paged to
uint32_t __declspec(naked) JumpToUpperHalf()
{
    __asm
    {
        pop eax                     // Pop return address off the stack
        sub eax, LOADBASE           // Change address to reflect where the kernel has been loaded in virtual memory
        add eax, BASE_ADDRESS
        push eax                    // Push the new return address back on the stack
        ret                         // Jump to the virtually-addressed kernel code
    }
}

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
    Paging_Enable(multibootInfo);

    // Fill terminal with 2's. We'll know paging works
    fill_term('2', VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    // Initialize global descriptor table
    GDT_Init();

    //uint32_t eipVal = 
    JumpToUpperHalf();
    //kprintf("eipVal: 0x%lX\n", eipVal);

    // Make sure KPageAllocator can find the kernel page directory
    tasks[0].cr3 = (uint32_t)pageDir;

    // Initialize graphical mode if Grub set one for us
    GraphicsInitFromGrub(multibootInfo);

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

    // Add kernel to list of tasks
    tasks[0].inUse = true;
    strncpy(tasks[0].imageName, "KERNEL PROCESS", sizeof("KERNEL PROCESS"));
    tasks[0].PID = nextPID - 1;

    // Initialize interrupts
    Interrupts_Init();

    init_serial();

    // Initialize the PCI bus
    PCI_Init();

    //Mouse_Init();

    // Execute autoexec.bat (if it exists)
    Autoexec();

    // Say Hello
    terminal_writestring("Hello world!\n");
    write_serial_string("\nHello world!\n");
    kprintf("Welcome to My OS (working title) build %d\n", BUILD_NUMBER);

    if (debugLevel)
    {
        terminal_writestring("LFB address: ");
        terminal_print_ulong_hex((uint32_t)multibootInfo->framebuffer_addr);
        terminal_newline();
    }

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

        if (showOverlay)
        {
            if (showClock)
            {
                TimeFormatTimeString(timeString);
                terminal_writestring_top(timeString, 61);
            }
            else
            {
                // Print the number of (non-keyboard) interrupts across the top of the screen
                terminal_writestring_top("Interrupts: ", 61);
                terminal_print_int_top(interrupts_fired, 73);
            }
        }

        if (mousePresent && showOverlay)
        {
            terminal_writestring_top("                      ", 0);

            if (mouseState.leftButton)
                terminal_writestring_top("L", 0);
            if (mouseState.middleButton)
                terminal_writestring_top("M", 1);
            if (mouseState.rightButton)
                terminal_writestring_top("R", 2);

            terminal_print_int_top(mouseState.mouseX, 3);
            terminal_writestring_top(",", 13);
            terminal_print_int_top(mouseState.mouseY, 14);

            if (!textMode && cursorEnabled)
            {
                int cursX = mouseState.mouseX;
                if (cursX < 0)
                    cursX = 0;
                if (cursX >= MAX_X_RES)
                    cursX = MAX_X_RES - 1;

                int cursY = mouseState.mouseY;
                if (cursY < 0)
                    cursY = 0;
                if (cursY >= MAX_Y_RES)
                    cursY = MAX_Y_RES - 1;

                // Restore backed-up image
                GraphicsBlit(oldMouseX, oldMouseY, (PIXEL_32BIT *)areaUnderCursor, 16, 16);
                //GraphicsPlotPixel(oldMouseX, oldMouseY, oldColor);

                // Backup current image
                //oldColor = GraphicsGetPixel(cursX, cursY);
                GraphicsCopyToImage(cursX, cursY, (PIXEL_32BIT *)areaUnderCursor, 16, 16);

                //GraphicsPlotPixel(cursX, cursY, graphicalForeground);

                GraphicsBlitWithAlpha(cursX, cursY, (PIXEL_32BIT *)cursorImage, 16, 16);

                oldMouseX = cursX;
                oldMouseY = cursY;
            }
        }

        // Just do nothing and wait for an interrupt
        __halt();
    }

}

// run autoexec.bat file, if it exists. Don't display error messages if it doesn't
void Autoexec(void)
{
    tftpHideErrors = true;

    OpenAndRunBatch("autoexec.bat");

    tftpHideErrors = false;
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
#if GRUB_GRAPHICS
    (uint32_t)GRAPHICS_MODE,
    (uint32_t)GRAPHICS_WIDTH,
    (uint32_t)GRAPHICS_HEIGHT,
    (uint32_t)GRAPHICS_DEPTH
#else
    (uint32_t)TEXT_MODE,
    (uint32_t)TEXT_WIDTH,
    (uint32_t)TEXT_HEIGHT,
    (uint32_t)TEXT_DEPTH    
#endif
};

#pragma comment(linker, "/merge:.text=.a")