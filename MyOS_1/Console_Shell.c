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

int inputPosition = 0;
#define COMMAND_HISTORY_SIZE        10
#define MAX_COMMAND_LENGTH          160

char commandHistory[COMMAND_HISTORY_SIZE][MAX_COMMAND_LENGTH];
char currentCommand[MAX_COMMAND_LENGTH];

#define MAX_SCREEN_CONTENTS  (80 * 25 + 32)
char screenContents[MAX_SCREEN_CONTENTS];


bool shellEnterPressed = false;

multiboot_info *multibootInfo = NULL;

// functions to manage a basic shell interface

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

#pragma warning(push)                 // disable warning message about divide-by-zero, because we do that intentionally with the crash command
#pragma warning(disable : 4723)       // (This is to test fault handling, which isn't actually implemented yet)
void Shell_Process_command()
{
    char subCommand[MAX_COMMAND_LENGTH];

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

        // restore screen contents
        if (wasTextMode)
        {
            // restore the text printed to the screen
            FNT_Render(screenContents);
            //terminal_writestring(screenContents);
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
        terminal_writestring("help\n");
        terminal_writestring("mbi\n");
        terminal_writestring("run [programName]\n");
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

    memset(subCommand, 0, MAX_COMMAND_LENGTH);
    strncpy(subCommand, currentCommand, strlen("debug"));
    if (strcmp(subCommand, "debug") == 0)
    {
        memset(subCommand, 0, MAX_COMMAND_LENGTH);
        strncpy(subCommand, currentCommand + strlen("debug "), MAX_COMMAND_LENGTH);
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

    if (strcmp(currentCommand, "dir") == 0)
    {
        // TODO: Improve, use dynamic memory
#define MAX_DIR_SIZE    2048
        uint8_t dirBuffer[MAX_DIR_SIZE + 1];
        memset(dirBuffer, 0, MAX_DIR_SIZE + 1);

        //TFTP_RequestFile(IPv4_PackIP(10,0,2,2), "dir.txt", TFTP_TYPE_BINARY, NIC_MAC);
        if (!TFTP_GetFile(IPv4_PackIP(10, 0, 2, 2), "dir.txt", dirBuffer, MAX_DIR_SIZE))
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

    if (currentCommand[0] == 'e' && currentCommand[1] == 'c' && currentCommand[2] == 'h' && currentCommand[3] == 'o')
    {
        strncpy(subCommand, currentCommand + 5, MAX_COMMAND_LENGTH - 5);
        terminal_writestring(subCommand);
        terminal_putchar('\n');
        return;
    }



    memset(subCommand, 0, MAX_COMMAND_LENGTH);
    strncpy(subCommand, currentCommand, strlen("run"));
    if (strcmp(subCommand, "run") == 0)
    {
        memset(subCommand, 0, MAX_COMMAND_LENGTH);
        strncpy(subCommand, currentCommand + strlen("run "), MAX_COMMAND_LENGTH);
        terminal_writestring("Ok, I'll run ");
        terminal_writestring(subCommand);
        terminal_newline();

        // TEMPTEMP we've hardcoded some memory starting at 0x800000. This was identity mapped when paging was enabled.
        uint8_t *exeBuffer = (uint8_t*)0x800000;
        //uint32_t bufferSize = 0x400000;

        uint32_t peBufferSize = 10 * 1024;

        if(debugLevel)
            terminal_dumpHex(peBuffer, 32);

        if (!TFTP_GetFile(IPv4_PackIP(10, 0, 2, 2), subCommand, peBuffer, peBufferSize) )
        {
            terminal_writestring("Error reading ");
            terminal_writestring(subCommand);
            terminal_writestring(" from server!\n");
            return;
        }

        if(debugLevel)
            terminal_dumpHex(peBuffer, 32);

        if (!loadAndRunPE(exeBuffer, (DOS_Header*)peBuffer))
            terminal_writestring("Error running executable\n");

        terminal_resume();
        terminal_writestring("done!\n");
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
    if(inputPosition)
        Shell_Process_command();

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
    currentCommand[inputPosition] = '\0';
    inputPosition--;
}