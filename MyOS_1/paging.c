#include "multiboot.h"
#include "Paging.h"
#include "Console_VGA.h"
#include <stdint.h>
#include "Interrupts/System_Calls.h"
#include "Tasks/Context.h"
#include "Console_Serial.h"

// Allocate space for paging structures. We need enough space for three arrays with 0x1000 32-bit entries each.
// Each array must be aligned on a 0x1000-byte boundary.
// I can't get alignment working via keywords, so I'll just allocate enough extra space to accomodate a 4096-aligned pointer.
// When we use these arrays, we'll need to subtract BASE_ADDRESS from their addresses and add LOADBASE, because paging hasn't been setup yet,
// and the linker will allocate them based on the given base address.
uint32_t paging_space[0x3FFF];

uint32_t *pageDir;

uint32_t pagingNextAvailableMemory;     // physical address of the next available page
uint32_t pagingNextAvailableKernelPage; // The index into the page directory where the next kernel page can go
uint32_t pagingKernelNonPagedArea;      // A page (4MiB) of non-paged, identity-mapped memory drivers can use (non-paged pool)
uint32_t paging4MPagesAvailable;        // number of 4M physical memory pages available to be mapped

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
    {
        //    pages = paging4MPagesAvailable;
        // NOPE: kmalloc expects all pages to be allocated
        kprintf("Too few pages available to satisfy request\n");
        return;
    }

    /*terminal_writestring("Need to allocate ");
    terminal_print_int(pages);
    terminal_writestring(" pages.\n");*/

    uint32_t nextPage = /*pagingNextAvailableKernelPage;// */pagingNextAvailableMemory / FOUR_MEGABYTES;
    uint32_t physicalAddress = pagingNextAvailableMemory;

    // sanity check
    if ((physicalAddress & PAGING_ADDRESS_BITS) != physicalAddress)
    {
        kprintf("Physical address does not lie on a 4M boundary!\nSystem halted.\n");
        for (;;)
            __halt();
    }

    // Get the page directory we'll be using
    PAGE_DIRECTORY_ENTRY *pageDirectory = (PAGE_DIRECTORY_ENTRY *)tasks[currentTask].cr3;

    // Add each page to the page directory
    for (size_t allocated = 0; allocated < pages; ++allocated)
    {
        // Add the current page to the page directory
        pageDirectory[nextPage] = ((physicalAddress)
            | DIRECTORY_ENTRY_PRESENT | DIRECTORY_ENTRY_USER_ACCESSIBLE | DIRECTORY_ENTRY_WRITABLE | DIRECTORY_ENTRY_4MB);

        // Since we're mapping this into the kernel space, we need to copy that mapping into every running task
        //if (pageDirectory == pageDir)
        {
            for (int i = 0; i < MAX_TASKS; ++i)
            {
                if (!tasks[i].inUse || tasks[i].cr3 == (uint32_t)pageDirectory)
                    continue;

                PAGE_DIRECTORY_ENTRY *otherPageDir = (PAGE_DIRECTORY_ENTRY *)tasks[i].cr3;

                // make sure we aren't overwriting the app's mapping (TODO: not sure how we are guaranteeing that we won't)
                if ((otherPageDir[nextPage] & DIRECTORY_ENTRY_PRESENT) == DIRECTORY_ENTRY_PRESENT)
                {
                    kprintf("Tried to overwrite page mapping of %s at 0x%X\nSystem halted.\n", tasks[i].imageName, nextPage * FOUR_MEGABYTES);
                    for (;;)
                        __halt();
                    // TODO: How should this be handled?
                }

                otherPageDir[nextPage] = ((physicalAddress)
                                           | DIRECTORY_ENTRY_PRESENT | DIRECTORY_ENTRY_USER_ACCESSIBLE | DIRECTORY_ENTRY_WRITABLE | DIRECTORY_ENTRY_4MB);
            }
        }
        
        // update pointers and stuff
        ++nextPage;
        ++pagingNextAvailableKernelPage;
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
    //serial_printf("Page table entries:\nLogical -> Physical  -- flags\n");
    for (int i = 0; i < 1024; ++i)
    {
        if (!(thePageDir[i] & PAGE_ENTRY_PRESENT))
            continue;
        done = false;
        kprintf("0x%X -> 0x%X   - 0x%X\n", i * FOUR_MEGABYTES, thePageDir[i] & 0xFFFFF000, thePageDir[i] & 0xFFF);
        //serial_printf("0x%X -> 0x%X   - 0x%X\n", i * FOUR_MEGABYTES, thePageDir[i] & 0xFFFFF000, thePageDir[i] & 0xFFF);
    }
    return done;
}

uint32_t Paging_Get_Physical_Address(void *address)
{
    uint32_t vAddr = (uint32_t)address;

    // TODO: don't always assume 4M entries

    uint32_t index = vAddr / FOUR_MEGABYTES;
    uint32_t offset = vAddr - (index * FOUR_MEGABYTES);

    // Get page directory
    PAGE_DIRECTORY_ENTRY *pDir = tasks[currentTask].cr3;

    uint32_t pAddr = pDir[index] & PAGING_ADDRESS_BITS;

    return pAddr + offset;
}
