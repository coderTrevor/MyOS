#include <stdint.h>
#include "Build_Number.h"
#include "Console_Shell.h"
#include "Console_VGA.h"
#include "misc.h"
#include "Networking/TFTP.h"
#include "Networking/IPv4.h"
#include "Executables/PE32.h"
#include "Graphics/Display_HAL.h"
#include "Drivers/Bochs_VGA.h"
#include "Graphics/picofont.h"
#include "Graphics/Graphical_Terminal.h"
#include "Terminal.h"
#include "Graphics/Bitmap.h"
#include "Interrupts/System_Calls.h"
#include "Timers/System_Clock.h"
#include "Timers/PIT.h"
#include "Drivers/Sound_Blaster_16.h"
#include "File Formats/VOC.h"
#include "Drivers/ISA_DMA.h"
#include "Drivers/AdLib.h"
#include "Executables/Batch_Files.h"
#include "paging.h"
#include "Drivers/Virtio_Net.h"
#include "Drivers/PS2_Mouse.h"
#include "Networking/DHCP.h"

int inputPosition = 0;
#define COMMAND_HISTORY_SIZE        10

char commandHistory[COMMAND_HISTORY_SIZE][MAX_COMMAND_LENGTH];
char currentCommand[MAX_COMMAND_LENGTH];

#define MAX_SCREEN_CONTENTS  (80 * 25 + 32)
char screenContents[MAX_SCREEN_CONTENTS];

// TODO: Improve the way history is indexed; don't overwrite old histories with new ones(?)
int historyIndex = -1;

bool shellEnterPressed = false;

multiboot_info *multibootInfo = NULL;

// functions to manage a basic shell interface

void Shell_Add_Command_To_History(char *command)
{
    // if the command is equal to the previous command, don't add it to the history
    if(historyIndex > 0 && strcmp(command, commandHistory[historyIndex-1]) == 0)
    {
        if(debugLevel)
            terminal_writestring("ignoring\n");
        return;
    }

    if (historyIndex < 0)
        historyIndex = 0;
    
    // copy the command to the command history
    strncpy(commandHistory[historyIndex], command, MAX_COMMAND_LENGTH);

    // increase history index, wrap around to the beginning if we reach the end
    if (++historyIndex == COMMAND_HISTORY_SIZE)
        historyIndex = 0;
}

void Shell_Erase_Current_Command()
{
    for (int i = 0; i < MAX_COMMAND_LENGTH; ++i)
    {
        currentCommand[i] = '\0';
    }
}

void PrintMultibootFlags(uint32_t flags)
{
    if (flags & MULTIBOOT_INFO_MEMORY)
        terminal_writestring("MULTIBOOT_INFO_MEMORY ");
    if (flags & MULTIBOOT_INFO_BOOTDEV)
        terminal_writestring("MULTIBOOT_INFO_BOOTDEV ");
    if (flags & MULTIBOOT_INFO_CMDLINE)
        terminal_writestring("MULTIBOOT_INFO_CMDLINE ");
    if (flags &  MULTIBOOT_INFO_MODS)
        terminal_writestring("MULTIBOOT_INFO_MODS ");

    if (flags & MULTIBOOT_INFO_AOUT_SYMS)
        terminal_writestring("MULTIBOOT_INFO_AOUT_SYMS ");
    if (flags & MULTIBOOT_INFO_ELF_SHDR)
        terminal_writestring("MULTIBOOT_INFO_ELF_SHDR ");

    if (flags & MULTIBOOT_INFO_MEM_MAP)
        terminal_writestring("MULTIBOOT_INFO_MEM_MAP ");

    if (flags & MULTIBOOT_INFO_DRIVE_INFO)
        terminal_writestring("MULTIBOOT_INFO_DRIVE_INFO ");

    if (flags & MULTIBOOT_INFO_CONFIG_TABLE)
        terminal_writestring("MULTIBOOT_INFO_CONFIG_TABLE ");

    if (flags & MULTIBOOT_INFO_BOOT_LOADER_NAME)
        terminal_writestring("MULTIBOOT_INFO_BOOT_LOADER_NAME ");

    if (flags & MULTIBOOT_INFO_APM_TABLE)
        terminal_writestring("MULTIBOOT_INFO_APM_TABLE ");

    if (flags & MULTIBOOT_INFO_VBE_INFO)
        terminal_writestring("MULTIBOOT_INFO_VBE_INFO ");
    if (flags &  MULTIBOOT_INFO_FRAMEBUFFER_INFO)
        terminal_writestring("MULTIBOOT_INFO_FRAMEBUFFER_INFO ");
}

void PrintMemMap()
{
    multiboot_mmap_entry *entry = (multiboot_mmap_entry *)multibootInfo->mmap_addr;
    if (debugLevel)
        terminal_dumpHex((uint8_t *)entry, multibootInfo->mmap_length);

    terminal_writestring("Address               Length                Type\n");
    // dump each mmap entry
    for (uint32_t offset = 0; offset <= multibootInfo->mmap_length && entry->size; offset += entry->size + 4)
    {
        terminal_print_ulonglong_hex(entry->addr);
        terminal_writestring("  ");
        terminal_print_ulonglong_hex(entry->len);
        terminal_writestring("  ");
        switch (entry->type)
        {
            case MULTIBOOT_MEMORY_AVAILABLE:
                terminal_writestring("AVAILABLE\n");
                break;
            case MULTIBOOT_MEMORY_RESERVED:
                terminal_writestring("RESERVED\n");
                break;
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                terminal_writestring("ACPI_RECLAIMABLE\n");
                break;
            case MULTIBOOT_MEMORY_NVS:
                terminal_writestring("MEMORY_NVS\n");
                break;
            case MULTIBOOT_MEMORY_BADRAM:
                terminal_writestring("BAD RAM\n");
                break;
            default:
                terminal_writestring("UNKNOWN TYPE\n");
        }

        // advance pointer to next entry
        entry = (multiboot_mmap_entry*)((uint32_t)entry + entry->size + 4);
    }
}

void *malloc(size_t);
void free(void *ptr);

#pragma warning(push)                 // disable warning message about divide-by-zero, because we do that intentionally with the crash command
#pragma warning(disable : 4723)       // (This is to test fault handling)
void Shell_Process_command(void)
{
    if (debugLevel)
    {
        terminal_writestring("Processing: ");
        terminal_writestring(currentCommand);
        terminal_newline();
    }

    char subCommand[MAX_COMMAND_LENGTH];

    // dhcp
    if (strcmp(currentCommand, "dhcp") == 0)
    {
        DHCP_Send_Discovery(NIC_MAC);
        return;
    }

    // Initialize mouse
    if (strcmp(currentCommand, "m") == 0)
    {
        Mouse_Init();
        return;
    }

    // Display kernel mapping
    if (strcmp(currentCommand, "base") == 0)
    {
        printf("Kernel is mapped to 0x%lX\n", BASE_ADDRESS);
        return;
    }

    // Check virtio-net receive queue
    if (strcmp(currentCommand, "scanrq") == 0)
    {
        VirtIO_Net_ScanRQ();
        return;
    }

    // Print info about memory allocations
    if (strcmp(currentCommand, "allocations") == 0)
    {
        terminal_writestring("Memory allocations:\n");
        bool some = false;

        for (size_t i = 0; i < nextAllocationSlot; ++i)
        {
            if (allocationArray.inUse[i])
            {
                terminal_print_ulong_hex(allocationArray.address[i]);
                terminal_writestring(": ");
                terminal_print_int(allocationArray.size[i]);
                terminal_writestring(" bytes\n");
                some = true;
            }
        }

        if (!some)
            terminal_writestring("none\n");
        return;
    }

    // Test memory allocation
    if (strcmp(currentCommand, "mem") == 0)
    {
        // print out some info about free memory
        uint32_t memFree = paging4MPagesAvailable * 4;
        terminal_print_int(memFree);
        terminal_writestring(" Megabytes available in unallocated pages\n");

        uint8_t *block = (uint8_t *)malloc(256);
        if (!block)
        {
            terminal_writestring("Out of available memory.\n");
            return;
        }
        for (unsigned int i = 0; i < 256; ++i)
        {
            block[i] = (uint8_t)i;
        }
        bool passed = true;

        for (unsigned int i = 0; i < 256; ++i)
        {
            if (block[i] != (uint8_t)i)
                passed = false;
        }

        terminal_writestring("Malloc(256) test ");
        if (passed)
            terminal_writestring("passed\n");
        else
            terminal_writestring("failed\n");

        uint16_t *block16 = (uint16_t *)malloc(65536);
        if (!block16)
        {
            terminal_writestring("Out of available memory.\n");
            return;
        }
        for (unsigned int i = 0; i < 65536 / sizeof(uint16_t); ++i)
        {
            block16[i] = (uint16_t)i;
        }
        passed = true;

        for (unsigned int i = 0; i < 65536 / sizeof(uint16_t); ++i)
        {
            if (block16[i] != (uint16_t)i)
                passed = false;
        }

        terminal_writestring("Malloc(64K) test ");
        if (passed)
            terminal_writestring("passed\n");
        else
            terminal_writestring("failed\n");

        // now try to allocate an entire page (4 megs)
        uint32_t *block32 = (uint32_t *)malloc(FOUR_MEGABYTES);
        if (!block32)
        {
            terminal_writestring("Out of available memory.\n");
            return;
        }

        for (unsigned int i = 0; i < FOUR_MEGABYTES / sizeof(uint32_t); ++i)
        {
            block32[i] = i;
        }

        passed = true;

        for (unsigned int i = 0; i < FOUR_MEGABYTES / sizeof(uint32_t); ++i)
        {
            if (block32[i] != i)
                passed = false;
        }

        terminal_writestring("Malloc(4M) test ");
        if (passed)
            terminal_writestring("passed\n");
        else
            terminal_writestring("failed\n");

        // Try to allocate 2 pages
        block32 = (uint32_t *)malloc(FOUR_MEGABYTES * 2);
        if (!block32)
        {
            terminal_writestring("Out of available memory.\n");
            return;
        }

        for (unsigned int i = 0; i < FOUR_MEGABYTES * 2 / sizeof(uint32_t); ++i)
        {
            block32[i] = i;
        }

        passed = true;

        for (unsigned int i = 0; i < FOUR_MEGABYTES * 2 / sizeof(uint32_t); ++i)
        {
            if (block32[i] != i)
                passed = false;
        }

        terminal_writestring("Malloc(8M) test ");
        if (passed)
            terminal_writestring("passed\n");
        else
            terminal_writestring("failed\n");

        // try to allocate a series of 64k blocks
        passed = true;
        for (unsigned int j = 0; j < 64; ++j)
        {
            block16 = (uint16_t *)malloc(65536);
            if (!block16)
            {
                terminal_writestring("Out of available memory.\n");
                return;
            }
            for (unsigned int i = 0; i < 65536 / sizeof(uint16_t); ++i)
            {
                block16[i] = (uint16_t)i;
            }

            for (unsigned int i = 0; i < 65536 / sizeof(uint16_t); ++i)
            {
                if (block16[i] != (uint16_t)i)
                    passed = false;
            }
        }

        terminal_writestring("Malloc(64K) * 64 test ");
        if (passed)
            terminal_writestring("passed\n");
        else
            terminal_writestring("failed\n");

        // print out some info about free memory
        memFree = paging4MPagesAvailable * 4;
        terminal_print_int(memFree);
        terminal_writestring(" Megabytes available in unallocated pages\n");

        // TODO: free() all allocated memory

        return;
    }

    // Test Adlib stuff
    if (strcmp(currentCommand, "adlib") == 0)
    {
        if (Adlib_Init())
            Adlib_Test();
        return;
    }

    // Test ISA DMA stuff
    if (strcmp(currentCommand, "dma") == 0)
    {
        DMA_InitBuffer();
        return;
    }

    // Test VOC playback
    if (strcmp(currentCommand, "play") == 0)
    {
        PlaySound(0);
        return;
    }
    if (strcmp(currentCommand, "play2") == 0)
    {
        PlaySound(1);
        return;
    }

    // test changing timer resolution
    if (strcmp(currentCommand, "highspeed") == 0)
    {
        PIT_Set_Interval(100);
        return;
    }

    // test VOC parsing
    if (strcmp(currentCommand, "voc") == 0)
    {
        OpenAndReadVOCs();
        return;
    }

    // test SB16 Init
    if (strcmp(currentCommand, "sb16") == 0)
    {
        SB16_Init();
        return;
    }

    // test system timer
    if (strcmp(currentCommand, "uptime") == 0)
    {
        int hours, minutes, seconds;
        TimeGetTimeSinceReset(&hours, &minutes, &seconds);
        
        terminal_writestring("System has been running for ");
        terminal_print_int(hours);
        terminal_writestring(" hours, ");
        terminal_print_int(minutes);
        terminal_writestring(" minutes, and ");
        terminal_print_int(seconds);
        terminal_writestring(" seconds.\n");
        return;
    }

    // test system calls
    if (strcmp(currentCommand, "syscall") == 0)
    {
        terminal_writestring("testing syscall\n");
        SystemCallPrint("test\n");
        return;
    }

    if (strcmp(currentCommand, "gfx") == 0)
    {
        if (!graphicsPresent)
        {
            terminal_writestring("Error: No recognized graphics adapter installed\n");
            return;
        }

        // backup text mode screen contents
        bool wasTextMode = textMode;
        if (textMode)
        {
            terminal_get_textmode_text(screenContents, MAX_SCREEN_CONTENTS);
        }

        // TODO: Switch to display HAL once that's implemented
        BGA_SetResolution(800, 600, 32);

        // Fill screen with color so we know it worked
        GraphicsFillScreen(0, 0, 0);
        if(debugLevel)
            GraphicsFillScreen(168, 68, 255);

        // Initialize graphical terminal
        GraphicalTerminalInit();

        // restore screen contents
        if (wasTextMode)
        {
            // restore the text printed to the screen
            terminal_writestring(screenContents);
        }

        return;
    }

    if (strcmp(currentCommand, "mbi") == 0)
    {
        // Print valid flags in multibootInfo
        terminal_writestring("Flags: ");
        PrintMultibootFlags(multibootInfo->flags);
        terminal_newline();
        terminal_newline();

        // Print boot loader name if given
        if (multibootInfo->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME)
        {
            terminal_writestring("Bootloader name: ");
            terminal_writestring((char *)multibootInfo->boot_loader_name);
            terminal_newline();
            terminal_newline();
        }

        // Print command line if given
        if (multibootInfo->flags & MULTIBOOT_INFO_CMDLINE)
        {
            terminal_writestring("Command line: ");
            terminal_writestring((const char *)multibootInfo->cmdline);
            terminal_newline();
            terminal_newline();
        }

        // Print memory info if given
        if (multibootInfo->flags & MULTIBOOT_INFO_MEMORY)
        {
            terminal_writestring("Memory:\nLower Mem: ");
            terminal_print_int(multibootInfo->mem_lower);
            terminal_writestring("\nUpper Mem: ");
            terminal_print_int(multibootInfo->mem_upper);
            terminal_newline();
            terminal_newline();
        }

        // Print memory map if given
        if (multibootInfo->flags & MULTIBOOT_INFO_MEM_MAP)
        {
            terminal_writestring("Memory map:\n");
            PrintMemMap();
            terminal_newline();
            terminal_newline();
        }
        return;
    }

    if (strcmp(currentCommand, "clear") == 0)
    {
        terminal_initialize();
        return;
    }

    if (strcmp(currentCommand, "help") == 0)
    {
        terminal_writestring("\nThe following commands are available:\n");
        terminal_writestring("clear\n");
        terminal_writestring("crash\n");
        terminal_writestring("debug [on|off]\n");
        terminal_writestring("dir\n");
        terminal_writestring("echo [string]\n");
        if(textMode && graphicsPresent)
            terminal_writestring("gfx\n");
        terminal_writestring("help\n");
        terminal_writestring("mbi\n");
        terminal_writestring("overlay [clock|ints|off]\n");
        terminal_writestring("run [programName]\n");
        if (!textMode)
            terminal_writestring("show [bitmapName]\n");
        terminal_writestring("uptime\n");
        terminal_writestring("ver\n\n");
        return;
    }

    if (strcmp(currentCommand, "crash") == 0)
    {
        terminal_writestring("About to divide-by-zero...\n");
        // we have to make the division complicated enough to trick the optimizer
        int i, j;
        for (i = 100; i < 200; ++i)
        {
            for (j = 0; j >= 0; --j)
            {
                terminal_print_int(i / j);
                terminal_newline();
            }
        }
        return;
    }

    if (strcmp(currentCommand, "dir") == 0)
    {
        // TODO: Improve, use dynamic memory
#define MAX_DIR_SIZE    2048
        uint8_t dirBuffer[MAX_DIR_SIZE + 1];
        memset(dirBuffer, 0, MAX_DIR_SIZE + 1);

        if (!TFTP_GetFile(tftpServerIP, "dir.txt", dirBuffer, MAX_DIR_SIZE, NULL))
        {
            terminal_writestring("Error reading dir.txt from server!\n");
            return;
        }

        // display the contents of dir.txt
        terminal_writestring((const char *)dirBuffer);

        return;
    }

    if (strcmp(currentCommand, "ver") == 0)
    {
        terminal_writestring("Build ");
        terminal_print_int(BUILD_NUMBER);
        terminal_writestring("\n\n");
        return;
    }

    // Commands with parameters:
    memset(subCommand, 0, MAX_COMMAND_LENGTH);

    // Debug command
    strncpy(subCommand, currentCommand, strlen("debug"));
    if (strcmp(subCommand, "debug") == 0)
    {
        memset(subCommand, 0, MAX_COMMAND_LENGTH);
        strncpy(subCommand, currentCommand + strlen("debug "), MAX_COMMAND_LENGTH - strlen("debug "));
        if (strcmp(subCommand, "on") == 0)
        {
            debugLevel = 9000;
            terminal_writestring("Debugging info set to maximum level.\n");
        }
        else
        {
            if (strcmp(subCommand, "off") == 0)
            {
                debugLevel = 0;
                terminal_writestring("Debugging info set to minimal level.\n");
            }
            else
                terminal_writestring("Usage: debug [on|off]\n");
        }
        return;
    }

    // Run command
    memset(subCommand, 0, MAX_COMMAND_LENGTH);
    strncpy(subCommand, currentCommand, strlen("run"));
    if (strcmp(subCommand, "run") == 0)
    {
        memset(subCommand, 0, MAX_COMMAND_LENGTH);
        strncpy(subCommand, currentCommand + strlen("run "), MAX_COMMAND_LENGTH - strlen("run "));

        if (strlen(subCommand) == 0)
        {
            terminal_writestring("You must specify the name of an executable to run!\n\nUsage: run [Name_Of_File.exe]\n");
            return;
        }

        if (debugLevel)
        {
            terminal_writestring("Ok, I'll run ");
            terminal_writestring(subCommand);
            terminal_newline();
        }

        // See if a batch file was requested
        if (IsBatchFile(subCommand))
        {
            OpenAndRunBatch(subCommand);
            if (debugLevel)
                terminal_writestring("done!\n");
            return;
        }

        // TEMPTEMP we've hardcoded some memory starting at 0x800000. This was identity mapped when paging was enabled.
        uint8_t *exeBuffer = (uint8_t*)0x800000;

        uint32_t peBufferSize = 10 * 1024;
        uint32_t peFileSize;

        if(debugLevel)
            terminal_dumpHex(peBuffer, 32);

        // Download the executable
        if (!TFTP_GetFile(tftpServerIP, subCommand, peBuffer, peBufferSize, &peFileSize) )
        {
            terminal_writestring("Error reading ");
            terminal_writestring(subCommand);
            terminal_writestring(" from server!\n");
            return;
        }

        if(debugLevel)
            terminal_dumpHex(peBuffer, 32);

        // Run the executable
        if (!loadAndRunPE(exeBuffer, (DOS_Header*)peBuffer))
            terminal_writestring("Error running executable\n");

        terminal_resume();

        if(debugLevel)
            terminal_writestring("done!\n");

        return;
    }

    // Bg command
    memset(subCommand, 0, MAX_COMMAND_LENGTH);
    strncpy(subCommand, currentCommand, strlen("bg"));
    if (strcmp(subCommand, "bg") == 0)
    {
        memset(subCommand, 0, MAX_COMMAND_LENGTH);
        strncpy(subCommand, currentCommand + strlen("bg "), MAX_COMMAND_LENGTH - strlen("bg "));

        if (textMode)
        {
            if (graphicsPresent)
                terminal_writestring("You must switch to graphical mode with the \"gfx\" command before setting a background.\n");
            else
                terminal_writestring("Sorry, I don't recognize your display adapter; I can't show you a bitmap.\n");
            return;
        }

        if (strlen(subCommand) == 0)
        {
            terminal_writestring("You must specify the name of a file to show!\n\nUsage: show [Name_Of_File.bmp]\n");
            return;
        }

        if (!SetGraphicalTerminalBackground(subCommand))
            return;

        GraphicsBlit(0, 0, backgroundImage, graphicsWidth, graphicsHeight);
        GraphicsBlitWithAlpha(0, 0, foregroundText, graphicsWidth, graphicsHeight);

        return;
    }

    // Show command
    memset(subCommand, 0, MAX_COMMAND_LENGTH);
    strncpy(subCommand, currentCommand, strlen("show"));
    if (strcmp(subCommand, "show") == 0)
    {
        memset(subCommand, 0, MAX_COMMAND_LENGTH);
        strncpy(subCommand, currentCommand + strlen("show "), MAX_COMMAND_LENGTH - strlen("show "));

        if (textMode)
        {
            if (graphicsPresent)
                terminal_writestring("You must switch to graphical mode with the \"gfx\" command before seeing a bitmap.\n");
            else
                terminal_writestring("Sorry, I don't recognize your display adapter; I can't show you a bitmap.\n");
            return;
        }

        if (strlen(subCommand) == 0)
        {
            terminal_writestring("You must specify the name of a file to show!\n\nUsage: show [Name_Of_File.bmp]\n");
            return;
        }

        PIXEL_32BIT *imageBuffer;
        uint32_t width, height;

        if (!Bitmap24Load(subCommand, &imageBuffer, &width, &height))
            return;

        // Display bitmap at bottom-right corner of the screen
        GraphicsBlit(graphicsWidth - width, graphicsHeight - height, imageBuffer, width, height);

        if(backgroundImage && foregroundText)
            GraphicsBlitToForeground(graphicsWidth - width, graphicsHeight - height, imageBuffer, width, height);

        free(imageBuffer);

        return;
    }

    // echo command
    memset(subCommand, 0, MAX_COMMAND_LENGTH);
    strncpy(subCommand, currentCommand, strlen("echo"));
    if (strcmp(subCommand, "echo") == 0)
    {
        memset(subCommand, 0, MAX_COMMAND_LENGTH);
        strncpy(subCommand, currentCommand + strlen("echo "), MAX_COMMAND_LENGTH - strlen("echo "));
        terminal_writestring(subCommand);
        terminal_putchar('\n');
        return;
    }

    // overlay command
    memset(subCommand, 0, MAX_COMMAND_LENGTH);
    strncpy(subCommand, currentCommand, strlen("overlay"));
    if (strcmp(subCommand, "overlay") == 0)
    {
        memset(subCommand, 0, MAX_COMMAND_LENGTH);
        strncpy(subCommand, currentCommand + strlen("overlay "), MAX_COMMAND_LENGTH - strlen("overlay "));
        if (strcmp(subCommand, "clock") == 0)
        {
            showOverlay = true;
            showClock = true;
            return;
        }
        if (strcmp(subCommand, "ints") == 0)
        {
            showOverlay = true;
            showClock = false;
            return;
        }
        if (strcmp(subCommand, "off") == 0)
        {
            showOverlay = false;
            return;
        }
        terminal_writestring("Sorry but I don't understand that parameter. Usage:\noverlay [clock|ints|off]\n");
        return;
    }

    // file size command
    memset(subCommand, 0, MAX_COMMAND_LENGTH);
    strncpy(subCommand, currentCommand, strlen("size"));
    if (strcmp(subCommand, "size") == 0)
    {
        memset(subCommand, 0, MAX_COMMAND_LENGTH);
        strncpy(subCommand, currentCommand + strlen("size "), MAX_COMMAND_LENGTH - strlen("size "));

        uint32_t fileSize = 0;

        if (!TFTP_GetFileSize(tftpServerIP, subCommand, &fileSize))
        {
            terminal_writestring("Couldn't get size of ");
            terminal_writestring(subCommand);
            terminal_newline();
            return;
        }

        terminal_writestring(subCommand);
        terminal_writestring(" is ");
        terminal_print_int(fileSize);
        terminal_writestring(" bytes\n");
        
        return;
    }

    terminal_writestring("Sorry, I don't know how to process that input.\n");
}
#pragma warning(pop)

void Shell_Enter_Pressed()
{
    shellEnterPressed = false;
    terminal_putchar('\n');
    
    // Handle current command
    if (inputPosition)
    {
        Shell_Add_Command_To_History(currentCommand);
        Shell_Process_command();
    }

    // reset command line
    Shell_Erase_Current_Command();
    inputPosition = 0;
    terminal_putchar('>');
}

void Shell_Key_Received(unsigned char key)
{
    if (key == '\b')
    {
        Shell_Backspace_Pressed();
        return;
    }

    if (key == '\r')
    {
        //Shell_Enter_Pressed();
        shellEnterPressed = true;
        return;
    }

    if (inputPosition >= MAX_COMMAND_LENGTH)
        return;

    // put the key on the screen
    terminal_putchar(key);
    currentCommand[inputPosition++] = key;
    //terminal_print_byte_hex(key);    
    //terminal_putchar(' ');
    //terminal_print_byte(0x9e);
}

void Shell_Delete_Pressed()
{

}

void Shell_Entry(multiboot_info *mbInfo)
{
    multibootInfo = mbInfo;

    inputPosition = 0;
    historyIndex = 0;

    terminal_writestring(">");
    // zero out command history
    for (int i = 0; i < COMMAND_HISTORY_SIZE; ++i)
    {
        for (int j = 0; j < MAX_COMMAND_LENGTH; ++j)
            commandHistory[i][j] = '\0';
    }
}

void Shell_Backspace_Pressed()
{
    if (inputPosition == 0)
        return;

    terminal_putchar('\b');
    currentCommand[--inputPosition] = '\0';
}

// TODO: Make less clunky
void Shell_Up_Pressed(void)
{
    if (debugLevel)
    {
        terminal_writestring("History index: ");
        terminal_print_int(historyIndex);
        terminal_newline();
    }

    // ensure there's some history to look at
    if (historyIndex <= 0)
        return;

    // get rid of whatever is typed on the command line
    Shell_Erase_Current_Command();
    while (inputPosition > 0)
    {
        terminal_putchar('\b');
        --inputPosition;
    }

    // replace current command with the previous one in history
    strncpy(currentCommand, commandHistory[--historyIndex], MAX_COMMAND_LENGTH);
    inputPosition = strlen(currentCommand);
    if (debugLevel)
    {
        terminal_writestring("inputPosition: ");
        terminal_print_int(inputPosition);
        terminal_newline();
    }
    
    terminal_writestring(commandHistory[historyIndex]);
}