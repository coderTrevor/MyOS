#include "multiboot.h"
#include "Paging.h"
#include "Console_VGA.h"
#include <stdint.h>

// Allocate space for paging structures. We need enough space for three arrays with 0x1000 32-bit entries each.
// Each array must be aligned on a 0x1000-byte boundary.
// I can't get alignment working via keywords, so I'll just allocate enough extra space to accomodate a 4096-aligned pointer.
// When we use these arrays, we'll need to subtract BASE_ADDRESS from their addresses and add LOADBASE, because paging hasn't been setup yet,
// and the linker will allocate them based on the given base address.
uint32_t paging_space[0x3FFF];

uint32_t *pageDir;

uint32_t pagingNextAvailableMemory; // physical address of the next available page
uint32_t paging4MPagesAvailable;    // number of 4M physical memory pages available to be mapped

// Allocates a number of 4-megabyte, consecutive pages. Why 4 megs? Because that's how MyOS rolls.
// We're just going to stick with identity mapping for now. Why? Because that's how MyOS rolls.
// Returns a pointer to the virtual address of the first allocated page or NULL if there was a problem.
// Due to memory usage / memory mapping, may allocate less than the requested number of pages.
// pPagesAllocated will receive the number of 4M pages actually allocated.
// TODO: Check memory map
// TODO: Make multi-threading safe
void *PageAllocator(unsigned int pages, unsigned int *pPagesAllocated)
{
    // make sure there's some pages available
    if (paging4MPagesAvailable == 0)
    {
        *pPagesAllocated = 0;
        return NULL;
    }

    void *retVal = (void *)pagingNextAvailableMemory;

    // limit the number of pages to allocate to the number available.
    if (pages < paging4MPagesAvailable)
        pages = paging4MPagesAvailable;

    uint32_t nextPage = pagingNextAvailableMemory / FOUR_MEGABYTES;

    // Add each page to the page directory
    for (size_t allocated = 0; allocated < pages; ++allocated)
    {
        // Add the current page to the page directory
        pageDir[nextPage] = ((nextPage * FOUR_MEGABYTES)
            | DIRECTORY_ENTRY_PRESENT | DIRECTORY_ENTRY_USER_ACCESSIBLE | DIRECTORY_ENTRY_WRITABLE | DIRECTORY_ENTRY_4MB);
        
        // update pointers and stuff
        ++nextPage;
        --paging4MPagesAvailable;
        pagingNextAvailableMemory += FOUR_MEGABYTES;
    }

    return retVal;
}