#include <stdint.h>
#include "misc.h"
#include "paging.h"
#include "Terminal.h"
#include "printf.h"
#include "Interrupts\System_Calls.h"
#include "Interrupts/Interrupts.h"

uint32_t pagedMemoryAvailable = 0;
uint32_t memoryNextAvailableAddress = 0;

// TODO: Change this to a more robust scheme (?)
ALLOCATION_ARRAY allocationArray = { 0 };
unsigned int nextAllocationSlot = 0;

// Very, very basic support for freeing memory:
ALLOCATION_ARRAY freeMemoryArray = { 0 };
unsigned int nextFreeMemorySlot = 0;

#ifdef DEBUG_MEM
#define noFileName "FILENAME NOT SET";
char *dbgMemFilename = noFileName;
int   dbgMemLineNumber = 0;
char *dbgFreeFilename = noFileName;
int   dbgFreeLineNumber = 0;
#endif 

void* calloc(size_t num, size_t size)
{
    if (num == 0 || size == 0)
        return 0;

    //printf("calloc called\n");

    // TODO: This will fail if num * size is greater than what size_t can represent
    uint8_t *mem = malloc(num * size);

    if (!mem)
        printf("     calloc failed! Couldn't allocate %d bytes\n", num * size);

    memset(mem, 0, num * size);

    return mem;
}

// TODO
void _exit(void)
{
    //terminal_writestring("Exit called with status \n");// %d\n", status);
    for (;;)
        __halt();
}


inline void addAllocationToFreeMemoryArray(int allocationIndex)
{
    // Try to add this memory to the free memory array
    if (nextFreeMemorySlot == MAX_ALLOCATIONS)
    {
        printf("No slots left for freed memory\n");
        return;
    }
    
    freeMemoryArray.address[nextFreeMemorySlot] = allocationArray.address[allocationIndex];
    freeMemoryArray.size[nextFreeMemorySlot] = allocationArray.size[allocationIndex];
    freeMemoryArray.inUse[nextFreeMemorySlot++] = true;
}

#ifdef DEBUG_MEM
void dbg_free(void *ptr, char *filename, int lineNumber)
{
    dbgFreeFilename = filename;
    dbgFreeLineNumber = lineNumber;
    free(ptr);
}
#endif

// TODO: Make thread-safe
void free(void *ptr)
{
    // Find this pointer in the allocation array
    for (size_t i = 0; i < nextAllocationSlot; ++i)
    {
        if (allocationArray.address[i] == (uint32_t)ptr)
        {
            if (allocationArray.inUse[i])
            {
                addAllocationToFreeMemoryArray(i);

                // We want to keep the allocation array from being fragmented, so we
                // copy the final entry in allocation array to the i position and
                // decrease the size of the allocation array
                --nextAllocationSlot;
                if (nextAllocationSlot)
                {
                    allocationArray.address[i] = allocationArray.address[nextAllocationSlot];
                    allocationArray.size[i] = allocationArray.size[nextAllocationSlot];
                }

                allocationArray.inUse[nextAllocationSlot] = false;
            }
            else
            {
                printf("free() called to free already-freed pointer, 0x%lX\n", ptr);
            }

            return;
        }
    }

    printf("free() called with invalid pointer: 0x%lX", ptr);
#ifdef DEBUG_MEM
    printf("   from %s, line %d\n", dbgFreeFilename, dbgFreeLineNumber);
    dbgFreeFilename = noFileName;
    dbgFreeLineNumber = 0;
    for (;;)
        __halt();
#else
    printf("\n");
#endif
}

char intToChar(int i)
{
    char *ints = "0123456789";
    if( i > 9 || i < 0)
        return '?';
    return ints[i];
}

// TODO: Test
void* realloc(void* ptr, size_t size)
{
    if (!ptr)
        return malloc(size);

    void *ptrNew = malloc(size);

    // Find ptr in allocation array
    int index;
    for (index = 0; index < MAX_ALLOCATIONS; ++index)
    {
        if (allocationArray.address[index] == (uint32_t)ptr)
            break;
    }

    // Could we find it?
    if (index < MAX_ALLOCATIONS)
    {
        size_t lesserSize = size;
        if (allocationArray.size[index] < size)
            lesserSize = allocationArray.size[index];

        // Copy the old data to the new memory
        memcpy(ptrNew, ptr, lesserSize);
    }

    free(ptr);

    return ptrNew;
}

char * __cdecl strchr(char *str, int character)
{
    char chr = (char)character;
    int currentPos = 0;
    
    // search each character of str for chr
    while (str[currentPos] != '\0')
    {
        if (str[currentPos] == chr)
            return &str[currentPos];

        ++currentPos;
    }

    return NULL;
}

// TODO: fix this
#pragma function(strcmp)
int __cdecl strcmp(const char *str1, const char *str2)
{
    while (*str1 != '\0')
    {
        if (*str1 != *str2)
            return *str1 - *str2;
        ++str1;
        ++str2;
    }

    if (*str2 != '\0')
        return *str2;

    return 0;
}

#pragma function(strlen)
size_t __cdecl strlen(const char* str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

// TODO: Test strncpy(), I'm not sure it's working right with regards to copying the terminator ('\0') (See RunBatch())
char * __cdecl strncpy(char *destination, const char *source, size_t num)
{
    size_t pos = 0;
    while (pos < num && *source != '\0')
    {
        *destination = *source;
        ++source;
        ++destination;
        ++pos;
    }
    
    // If source string is less than num characters long, fill the remainder of destination with 0's
    while (pos < num)
    {
        *destination = '\0';
        ++destination;
        ++pos;
    }

    return destination;
}

#ifdef DEBUG_MEM
void *dbg_malloc(size_t size, char *filename, int lineNumber)
{
    dbgMemFilename = filename;
    dbgMemLineNumber = lineNumber;
    return malloc(size);
}
#endif

unsigned int reuses = 0;
void* malloc(size_t size)
{
    if (nextAllocationSlot >= MAX_ALLOCATIONS)
    {
        printf("Maximum memory allocations exceeded!\n");
        return NULL;
    }

#ifdef DEBUG_MEM
    allocationArray.lineNumber[nextAllocationSlot] = dbgMemLineNumber;
    strncpy(allocationArray.filename[nextAllocationSlot], dbgMemFilename, MAX_DEBUG_FILENAME_LENGTH);
    dbgMemFilename = noFileName;
    dbgMemLineNumber = 0;
#endif


    // See if there's freed memory available to reallocate (first fit algorithm; memory will end up wasted)
    for(size_t i = 0; i < nextFreeMemorySlot; ++i)
    {
        if (freeMemoryArray.size[i] >= size)
        {
            // We found a piece of free memory we can reuse

            // Keep track of the memory in our allocations array
            allocationArray.address[nextAllocationSlot] = freeMemoryArray.address[i];
            allocationArray.size[nextAllocationSlot] = freeMemoryArray.size[i];
            allocationArray.inUse[nextAllocationSlot] = true;

            // We want to keep freeMemoryArray from fragmenting, so we'll copy
            // last used free memory entry of the array to the i position and decrease
            // the used portion of the array by one
            --nextFreeMemorySlot;
            if (nextFreeMemorySlot)
            {
                freeMemoryArray.address[i] = freeMemoryArray.address[nextFreeMemorySlot];
                freeMemoryArray.size[i] = freeMemoryArray.size[nextFreeMemorySlot];
            }
            // The last entry is no longer in use
            freeMemoryArray.inUse[nextFreeMemorySlot] = false;

#ifdef DEBUG_MEM
            printf("Reusing freed memory from slot %d, (reuse #%d)\n", i, ++reuses);
#endif
            return (void *)allocationArray.address[nextAllocationSlot++];
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

        /*terminal_writestring("Need to allocate ");
        terminal_print_int(pagesToAllocate);
        terminal_writestring(" pages.\n");*/

        // check for remainder from division
        if (pagesToAllocate * FOUR_MEGABYTES < size)
        {
            ++pagesToAllocate;
            /*terminal_writestring("...I mean ");
            terminal_print_int(pagesToAllocate);
            terminal_writestring(" pages.\n");*/
        }

        // Allocate the pages
        // This will be virtual memory (sort of, we use identity-mapping for now but that's temporary)
        // So we can guarantee the pages will be allocated in order. (TODO: Now that I think of it, this logic should maybe be in PageAllocator?)
        /*while (pagesToAllocate)
        {
            unsigned int pagesAllocated;
            PageAllocator(pagesToAllocate, &pagesAllocated);
            pagesToAllocate -= pagesAllocated;
        }*/

        unsigned int pagesAllocated;
        // Run KPageAllocator() in kernel mode, otherwise use a system call from applications
#ifndef MYOS_KERNEL
        pageAllocator(pagesToAllocate, &pagesAllocated, &availableAddress);
#else
        KPageAllocator(pagesToAllocate, &pagesAllocated, &availableAddress);
#endif
        if (!availableAddress)
        {
            printf("Returning NULL, %d pages allocated out of %d\n", pagesAllocated, pagesToAllocate);
            return NULL;
        }

        /*terminal_writestring("Just allocated ");
        terminal_print_int(pagesAllocated);
        terminal_writestring(" pages\n");*/

        // TODO: see if the page we allocated follows the previous page
        // for now, we'll just ignore the old allocated memory
        pagedMemoryAvailable = pagesAllocated * FOUR_MEGABYTES;

        memoryNextAvailableAddress = availableAddress;
    }

    pagedMemoryAvailable -= size;
    availableAddress = memoryNextAvailableAddress;
    memoryNextAvailableAddress += size;

    // Keep track of the memory in our allocations array
    allocationArray.address[nextAllocationSlot] = availableAddress;
    allocationArray.size[nextAllocationSlot] = size;
    allocationArray.inUse[nextAllocationSlot++] = true;

    return (void *)availableAddress;
}

#pragma function(memcmp)
int __cdecl memcmp(const void *ptr1, const void *ptr2, size_t num)
{
    uint8_t *p1 = (uint8_t *)ptr1;
    uint8_t *p2 = (uint8_t *)ptr2;

    for (size_t i = 0; i < num; ++i)
    {
        if (p1[i] != p2[i])
            return p2[i] - p1[i];
    }

    return 0;
}

#pragma function(memcpy)
void * __cdecl memcpy(void *dest, const void *src, size_t count)
{
    uint8_t *dst = dest;
    const uint8_t *source = src;

    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = source[i];
    }

    return dst;
}

#pragma function(memset)
void * __cdecl memset(void *ptr, int value, size_t num)
{
    uint8_t *dst = ptr;
    for (size_t i = 0; i < num; ++i)
    {
        *dst = (uint8_t)value;
        ++dst;
    }

    return dst;
}

void showAllocations(void)
{
    printf("Memory allocations:\n");

    for (size_t i = 0; i < nextAllocationSlot; ++i)
    {
        if (allocationArray.inUse[i])
        {
            printf("0x%lX: %d bytes", allocationArray.address[i], allocationArray.size[i]);
            /*terminal_print_ulong_hex(allocationArray.address[i]);
            terminal_writestring(": ");
            terminal_print_int(allocationArray.size[i]);
            terminal_writestring(" bytes");*/

#ifdef DEBUG_MEM
            printf("   from %s, line %d\n", allocationArray.filename[i], allocationArray.lineNumber[i]);
#else
            printf("\n");
#endif

        }
        else
            printf("Allocated memory index %d is marked not in use\n", i);
    }

    if (nextAllocationSlot)
        printf("%d total allocations\n", nextAllocationSlot);
    else
        printf("none\n");
}