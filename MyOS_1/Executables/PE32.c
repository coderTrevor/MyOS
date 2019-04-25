
#include <stdbool.h>
#include "../misc.h"
#include "PE32.h"
#include "../Terminal.h"

// TEMPTEMP - until we have memory allocation, just reserve 10k for an executable
uint8_t peBuffer[10 * 1024];


// returns true on success
bool loadAndRunPE(uint8_t *executableDestination, DOS_Header *mzAddress)
{
    // Ensure the file starts with a 'mz' signature
    if (mzAddress->signature != DOS_MAGIC)
    {
        terminal_writestring("Error: not a valid PE file!\n");
        return false;
    }

    // Find the PE Header structure
    uint32_t headerOffset = mzAddress->e_lfanew;
    if (debugLevel)
    {
        terminal_writestring("PE header at ");
        terminal_print_ulong_hex(headerOffset);
        terminal_newline();
    }

    PE_Header *peHeader = (PE_Header*)((uint32_t)mzAddress + headerOffset);

    if (debugLevel)
        terminal_dumpHex((uint8_t*)peHeader, 32);

    // Ensure PE Header starts with the magic number
    if (peHeader->mMagic != PE_MAGIC)
    {
        terminal_writestring("File doesn't contain a valid PE Header!\n");
        return false;
    }

    // Check for the presence of the "optional" header
    if (peHeader->mSizeOfOptionalHeader == 0)
    {
        terminal_writestring("PE Optional header not found!\n");
        return false;
    }

    // Get the address of the "optional" header
    PE32OptionalHeader *peOptionalHeader = (PE32OptionalHeader*)((uint32_t)peHeader + sizeof(PE_Header));
    if (peOptionalHeader->mMagic != PE32_MAGIC)
    {
        terminal_writestring("Don't know how to load PE of type ");
        terminal_print_ushort_hex(peOptionalHeader->mMagic);
        terminal_newline();
        return false;
    }

    // Skip over the data directories to find the end of the optional header, and the beginning of the sections
    IMAGE_DATA_DIRECTORY *directory = NULL;
    for (uint32_t i = 0; i < peOptionalHeader->mNumberOfRvaAndSizes; ++i)
    {
        directory = &peOptionalHeader->mDataDirectories[i];

        if (debugLevel)
        {
            terminal_print_ulong_hex(directory->VirtualAddress);
            terminal_writestring(" [");
            terminal_print_ulong_hex(directory->Size);
            terminal_writestring("]\n");
        }
    }

    // TEMPTEMP - zero out 5 0x1000 sections of memory (tailored to TestApp1.exe)
    memset(executableDestination, 0, 0x5000);

    // Get the address of the first section
    IMAGE_SECTION_HEADER *sectionHeader = (IMAGE_SECTION_HEADER*)((uint32_t)directory + sizeof(IMAGE_DATA_DIRECTORY));

    // Copy each section into memory
    for (int i = 0; i < peHeader->mNumberOfSections; ++i)
    {
        if (debugLevel)
        {
            terminal_writestring("Section ");
            terminal_print_int(i);
            terminal_writestring(":\n   ");
            terminal_writestring(sectionHeader->mName);
            terminal_newline();
        }
        
        // Determine the destination of the current section
        uint8_t *sectionDest = (uint8_t*)((uint32_t)executableDestination + sectionHeader->mVirtualAddress);
        if (debugLevel)
        {
            terminal_writestring("Virtual address of section: ");
            terminal_print_ulong_hex((uint32_t)sectionDest);
            terminal_newline();

            terminal_writestring("Size of raw data: ");
            terminal_print_ulong_hex(sectionHeader->mSizeOfRawData);
            terminal_newline();
        }

        // Determine source address of the the current section
        uint8_t *rawDataPtr = (uint8_t*)((uint32_t)mzAddress + sectionHeader->mPointerToRawData);
        if (debugLevel)
        {
            terminal_writestring("Pointer to raw data: ");
            terminal_print_ulong_hex((uint32_t)rawDataPtr);
            terminal_newline();
        }

        // Copy the current section to its destination
        memcpy(sectionDest, rawDataPtr, sectionHeader->mSizeOfRawData);

        // Advance to the next section
        sectionHeader = (IMAGE_SECTION_HEADER*)((uint32_t)sectionHeader + sizeof(IMAGE_SECTION_HEADER));
    }

    // Determine the entry point of the PE (we only support int main(void) right now)
    int(*entryPoint)(void) = (int(*)(void))((uint32_t)executableDestination + peOptionalHeader->mAddressOfEntryPoint);
    if (debugLevel)
    {
        terminal_writestring("Address of entry: ");
        terminal_print_ulong_hex((uint32_t)entryPoint);
        terminal_newline();
    }

    // Run the program
    (*entryPoint)();

    return true;
}