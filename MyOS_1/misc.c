#include <stdint.h>
#include "misc.h"
#include "paging.h"
#include "Terminal.h"
#include "printf.h"
#include "Interrupts\System_Calls.h"

uint32_t pagedMemoryAvailable = 0;
uint32_t memoryNextAvailableAddress = 0;

ALLOCATION_ARRAY allocationArray = { 0 };
unsigned int nextAllocationSlot = 0;

void* calloc(size_t num, size_t size)
{
    if (num == 0 || size == 0)
        return 0;

    // TODO: This will fail if num * size is greater than what size_t can represent
    uint8_t *mem = malloc(num * size);

    memset(mem, 0, num * size);

    return mem;
}

// TODO
void _exit()
{
    //terminal_writestring("Exit called with status \n");// %d\n", status);
    for (;;)
        __halt();
}

void free(void *ptr)
{
    // TODO: Implement free() properly
    bool found = false;
    for (size_t i = 0; i < nextAllocationSlot; ++i)
    {
        if (allocationArray.address[i] == (uint32_t)ptr)
        {
            if (allocationArray.inUse[i])
            {
                allocationArray.inUse[i] = false;
                found = true;
            }
            else
            {
                /*terminal_writestring("free() called to free already-freed pointer, ");
                terminal_print_ulong_hex((uint32_t)ptr);
                terminal_newline();*/
                printf("free() called to free already-freed pointer, %lX\n");
            }
        }
    }

    if (!found)
    {
        /*terminal_writestring("free() called with invalid pointer: ");
        terminal_print_ulong_hex((uint32_t)ptr);
        terminal_newline();*/
        printf("free() called with invalid pointer: %lX\n");
    }
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

void* malloc(size_t size)
{
    if (nextAllocationSlot >= MAX_ALLOCATIONS)
    {
        printf("Maximum memory allocations exceeded!\n");
        return NULL;
    }

    uint32_t availableAddress = memoryNextAvailableAddress;

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
        availableAddress = (uint32_t)(PageAllocator(pagesToAllocate, &pagesAllocated));
        if (!availableAddress)
            return NULL;

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