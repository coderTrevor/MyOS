#include "paging.h"
#include "printf.h"
#include "Interrupts\System_Calls.h"
#include "kmisc.h"
#include "misc.h"
#include "Console_Serial.h"
#include "Tasks/Context.h"

// Functions used by the kernel & drivers

// TODO: This might all go out the window in favor of conditionally-compiling misc.c functions; I haven't decided yet.

extern uint32_t memoryNextAvailableAddress;
extern uint32_t pagedMemoryAvailable;

// TODO: Change this to a more robust scheme (?)
ALLOCATION_ARRAY kallocationArray = { 0 };
unsigned int knextAllocationSlot = 0;

// Very, very basic support for freeing memory:
ALLOCATION_ARRAY kfreeMemoryArray = { 0 };
unsigned int knextFreeMemorySlot = 0;

#ifdef DEBUG_MEM
#define noFileName "FILENAME NOT SET";
char *dbgkMemFilename = noFileName;
int   dbgkMemLineNumber = 0;
char *dbgkFreeFilename = noFileName;
int   dbgkFreeLineNumber = 0;
#endif 

inline void addAllocationToKFreeMemoryArray(int allocationIndex)
{
    // Try to add this memory to the free memory array
    if (knextFreeMemorySlot == MAX_ALLOCATIONS)
    {
        kprintf("NSLFM");
        return;
    }

    kfreeMemoryArray.address[knextFreeMemorySlot] = kallocationArray.address[allocationIndex];
    kfreeMemoryArray.size[knextFreeMemorySlot] = kallocationArray.size[allocationIndex];
    kfreeMemoryArray.inUse[knextFreeMemorySlot++] = true;
}

#ifdef DEBUG_MEM
void *dbg_kmalloc(size_t size, char *filename, int lineNumber)
{
    dbgkMemFilename = filename;
    dbgkMemLineNumber = lineNumber;
    serial_printf("Allocating %d bytes from %s, line %d for %s\n", size, filename, lineNumber, tasks[currentTask].imageName);
    return kmalloc(size);
}
#endif

// Allocate some kernel memory. Mostly this is meant to be used by drivers.
// This will be mapped into every tasks page space and will be allocated contiguously
// TODO: Is it better to have this crazy scheme with the two functions, or just modify malloc to make sure
// it never reuses user-space memory when called from the kernel?

void* kmalloc(size_t size)
{    
    if (knextAllocationSlot >= MAX_ALLOCATIONS)
    {
        kprintf("Maximum memory allocations exceeded!\n");
        return NULL;
    }

#ifdef DEBUG_MEM
    kallocationArray.lineNumber[knextAllocationSlot] = dbgkMemLineNumber;
    strncpy(kallocationArray.filename[knextAllocationSlot], dbgkMemFilename, MAX_DEBUG_FILENAME_LENGTH);
    dbgkMemFilename = noFileName;
    dbgkMemLineNumber = 0;
#endif

    // See if there's freed memory available to reallocate (first fit algorithm; memory will end up wasted)
    for (size_t i = 0; i < knextFreeMemorySlot; ++i)
    {
        if (kfreeMemoryArray.size[i] >= size)
        {
            // We found a piece of free memory we can reuse

            // Keep track of the memory in our allocations array
            kallocationArray.address[knextAllocationSlot] = kfreeMemoryArray.address[i];
            kallocationArray.size[knextAllocationSlot] = kfreeMemoryArray.size[i];
            kallocationArray.inUse[knextAllocationSlot] = true;

            // We want to keep kfreeMemoryArray from fragmenting, so we'll copy
            // last used free memory entry of the array to the i position and decrease
            // the used portion of the array by one
            --knextFreeMemorySlot;
            if (knextFreeMemorySlot)
            {
                kfreeMemoryArray.address[i] = kfreeMemoryArray.address[knextFreeMemorySlot];
                kfreeMemoryArray.size[i] = kfreeMemoryArray.size[knextFreeMemorySlot];
            }
            // The last entry is no longer in use
            kfreeMemoryArray.inUse[knextFreeMemorySlot] = false;

#ifdef DEBUG_MEM
            //printf("Reusing freed memory from slot %d, (reuse #%d)\n", i, ++reuses);
#endif
            return (void *)kallocationArray.address[knextAllocationSlot++];
        }
    }

    uint32_t availableAddress = memoryNextAvailableAddress;

#ifdef DEBUG_MEM
    printf("Allocating new %d bytes\n", size);
#endif

    //if(debugLevel)
    //printf("size: %d\nadrress: %d\n", size, memoryNextAvailableAddress);

    /*terminal_writestring("Paged memory available: ");
    terminal_print_int(pagedMemoryAvailable);
    terminal_newline();*/

    // see if we need to allocate a page
    if (size > pagedMemoryAvailable)
    {
        // TODO: Support dynamic page granularity, not just large pages
        unsigned int pagesToAllocate = size / FOUR_MEGABYTES;

        // check for remainder from division
        if (pagesToAllocate * FOUR_MEGABYTES < size)
            ++pagesToAllocate;

        // Allocate the pages
        unsigned int pagesAllocated;        
        KPageAllocator(pagesToAllocate, &pagesAllocated, &availableAddress);

        // We need to ensure the pages are in order
        if (!availableAddress || (pagesAllocated < pagesToAllocate))
        {
            kprintf("Returning NULL, %d pages allocated out of %d\n", pagesAllocated, pagesToAllocate);
            // TODO: Free allocated pages
            return NULL;
        }

        // TODO: see if the page we allocated follows the previous page
        // for now, we'll just ignore the old allocated memory
        pagedMemoryAvailable = pagesAllocated * FOUR_MEGABYTES;

        memoryNextAvailableAddress = availableAddress;
    }

    pagedMemoryAvailable -= size;
    availableAddress = memoryNextAvailableAddress;
    memoryNextAvailableAddress += size;

    // Keep track of the memory in our allocations array
    kallocationArray.address[knextAllocationSlot] = availableAddress;
    kallocationArray.size[knextAllocationSlot] = size;
    kallocationArray.inUse[knextAllocationSlot++] = true;

    return (void *)availableAddress;
}

#ifdef DEBUG_MEM
void dbg_kfree(void *ptr, char *filename, int lineNumber)
{
    dbgkFreeFilename = filename;
    dbgkFreeLineNumber = lineNumber;
    serial_printf("Freeing mem from %s, line %d for %s\n", filename, lineNumber, tasks[currentTask].imageName);
    kfree(ptr);
}
#endif

void kfree(void *ptr)
{
    // Find this pointer in the allocation array
    for (size_t i = 0; i < knextAllocationSlot; ++i)
    {
        if (kallocationArray.address[i] == (uint32_t)ptr)
        {
            if (kallocationArray.inUse[i])
            {
                addAllocationToKFreeMemoryArray(i);

                // We want to keep the allocation array from being fragmented, so we
                // copy the final entry in allocation array to the i position and
                // decrease the size of the allocation array
                --knextAllocationSlot;
                if (knextAllocationSlot)
                {
                    kallocationArray.address[i] = kallocationArray.address[knextAllocationSlot];
                    kallocationArray.size[i] = kallocationArray.size[knextAllocationSlot];
                }

                kallocationArray.inUse[knextAllocationSlot] = false;
            }
            else
            {
                kprintf("free() called to free already-freed pointer, 0x%lX\n", ptr);
            }

            return;
        }
    }

    kprintf("free() called with invalid pointer: 0x%lX", ptr);
#ifdef DEBUG_MEM
    printf("   from %s, line %d\n", dbgkFreeFilename, dbgkFreeLineNumber);
    dbgkFreeFilename = noFileName;
    dbgkFreeLineNumber = 0;
    //for (;;)
    //    __halt();
#else
    kprintf("\n");
#endif
}