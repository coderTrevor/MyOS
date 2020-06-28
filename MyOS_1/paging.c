#include "multiboot.h"
#include "Paging.h"
#include "Console_VGA.h"
#include <stdint.h>
#include "Interrupts/System_Calls.h"
#include "Tasks/Context.h"

// Allocate space for paging structures. We need enough space for three arrays with 0x1000 32-bit entries each.
// Each array must be aligned on a 0x1000-byte boundary.
// I can't get alignment working via keywords, so I'll just allocate enough extra space to accomodate a 4096-aligned pointer.
// When we use these arrays, we'll need to subtract BASE_ADDRESS from their addresses and add LOADBASE, because paging hasn't been setup yet,
// and the linker will allocate them based on the given base address.
uint32_t paging_space[0x3FFF];

uint32_t *pageDir;

uint32_t pagingNextAvailableMemory; // physical address of the next available page
uint32_t paging4MPagesAvailable;    // number of 4M physical memory pages available to be mapped

uint32_t nextPageDirectory; // TEMPTEMP where the next created page directory will be stored

// Allocates a number of 4-megabyte, consecutive pages. Why 4 megs? Because that's how MyOS rolls.
// We're just going to stick with identity mapping for now. Why? Because that's how MyOS rolls.
// Returns a pointer to the virtual address of the first allocated page or NULL if there was a problem.
// Due to memory usage / memory mapping, may allocate less than the requested number of pages.
// pPagesAllocated will receive the number of 4M pages actually allocated.
// TODO: Check memory map
// TODO: Make multi-threading safe
void KPageAllocator(unsigned int pages, unsigned int *pPagesAllocated, uint32_t *pRetVal)
{
    // Ensure interrupts are disabled
    __asm
    {
        pushf
        cli
    }

    *pPagesAllocated = 0;

    // make sure there's enough pages available
    if (paging4MPagesAvailable < pages)
    {
        printf("Not enough pages available, %d out of %d\n", paging4MPagesAvailable, pages);
        *pRetVal = (uint32_t)NULL;
        return;
    }

    *pRetVal = pagingNextAvailableMemory;

    // limit the number of pages to allocate to the number available.
    if (pages > paging4MPagesAvailable)
        pages = paging4MPagesAvailable;

    /*terminal_writestring("Need to allocate ");
    terminal_print_int(pages);
    terminal_writestring(" pages.\n");*/

    uint32_t nextPage = pagingNextAvailableMemory / FOUR_MEGABYTES;

    // Get the page directory we'll be using
    PAGE_DIRECTORY_ENTRY *pageDirectory = (PAGE_DIRECTORY_ENTRY *)tasks[currentTask].cr3;

    // Add each page to the page directory
    for (size_t allocated = 0; allocated < pages; ++allocated)
    {
        // Add the current page to the page directory
        pageDirectory[nextPage] = ((nextPage * FOUR_MEGABYTES)
            | DIRECTORY_ENTRY_PRESENT | DIRECTORY_ENTRY_USER_ACCESSIBLE | DIRECTORY_ENTRY_WRITABLE | DIRECTORY_ENTRY_4MB);

        // If we're mapping this into the kernel space, we need to copy that mapping into every running task
        if (pageDirectory == pageDir)
        {
            for (int i = 0; i < MAX_TASKS; ++i)
            {
                if (!tasks[i].inUse || tasks[i].cr3 == pageDir)
                    continue;

                PAGE_DIRECTORY_ENTRY *otherPageDir = (PAGE_DIRECTORY_ENTRY *)tasks[i].cr3;

                otherPageDir[nextPage] = ((nextPage * FOUR_MEGABYTES)
                                           | DIRECTORY_ENTRY_PRESENT | DIRECTORY_ENTRY_USER_ACCESSIBLE | DIRECTORY_ENTRY_WRITABLE | DIRECTORY_ENTRY_4MB);
            }
        }
        
        // update pointers and stuff
        ++nextPage;
        --paging4MPagesAvailable;
        pagingNextAvailableMemory += FOUR_MEGABYTES;
        ++(*pPagesAllocated);
    }

    //kprintf("Task: %s\n", tasks[currentTask].imageName);
    //Paging_Print_Page_Table(pageDirectory);

    __asm popf
}

bool Paging_Print_Page_Table(PAGE_DIRECTORY_ENTRY *thePageDir)
{
    bool done;
    done = true;

    kprintf("Page table entries:\nLogical -> Physical  -- flags\n");
    kprintf("continuing\n");
    for (int i = 0; i < 1024; ++i)
    {
        if (!(thePageDir[i] & PAGE_ENTRY_PRESENT))
            continue;
        done = false;
        kprintf("0x%X -> 0x%X   - 0x%X\n", i * FOUR_MEGABYTES, thePageDir[i] & 0xFFFFF000, thePageDir[i] & 0xFFF);
    }
    return done;
}
