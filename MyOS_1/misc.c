#include <stdint.h>
#include "misc.h"
#include "paging.h"
#include "Terminal.h"
#include "printf.h"
#include "Interrupts\System_Calls.h"
#include "Interrupts/Interrupts.h"
#include "Console_Serial.h"
#include <float.h>
#include "Tasks/Context.h"

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
    serial_printf("Freeing mem from %s, line %d for %s\n", filename, lineNumber, tasks[currentTask].imageName);
    free(ptr);
}
#endif

// TODO: Make thread-safe
void free(void *ptr)
{
#ifdef MYOS_KERNEL
    kfree(ptr);
    return;
#else
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
    //for (;;)
    //    __halt();
#else
    printf("\n");
#endif

#endif // MYOS_KERNEL
}

char intToChar(int i)
{
    char *ints = "0123456789";
    if( i > 9 || i < 0)
        return '?';
    return ints[i];
}

// TODO: Test
// TODO: Handle kernel-mode
void* realloc(void* ptr, size_t size)
{
    if (!ptr)
        return malloc(size);

    void *ptrNew = malloc(size);

#ifdef MYOS_KERNEL
    serial_printf("realloc called\n");
#endif

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

// Globals for setjmp / longjmp
// TODO: Actually use jmp_buf struct and not these hacky globals
uint32_t gebpVal;
uint32_t gebxVal;
uint32_t gediVal;
uint32_t geipVal;
uint32_t gesiVal;
uint32_t gespVal;

// TODO: use env instead of global variables
int __declspec(naked) longjmp(jmp_buf env, int val)
{
    __asm
    {
        mov ebp, gebpVal    // restore ebp, ebx, edi, esi, and esp registers
        mov ebx, gebxVal
        mov edi, gediVal
        mov esi, gesiVal
        mov esp, gespVal    

        push geipVal        // push stored return address to the stack (longjmp will return to this address when ret is executed)

        mov eax, val        // return val
        sti                 // re-enable interrupts (they were never re-enabled from the exit_interrupt_handler)
        ret
    }
}

// TODO: Store these values in buf, not global variables
int __declspec(naked) setjmp(jmp_buf buf)
{
    __asm
    {
        mov gebpVal, ebp        // store ebp, ebx, and edi
        mov gebxVal, ebx
        mov gediVal, edi

        pop eax                 // store return value in geipVal
        mov geipVal, eax
        push eax

        mov gesiVal, esi        // save esi in gesiVal

        lea ecx, [esp + 8]      // Get value of esp before setjmp call
        mov gespVal, ecx
    
        xor eax, eax            // clear eax (return 0)
        ret
    }
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

char * __cdecl strdup(const char *str)
{
    size_t size = strlen(str) + 1;
    char *newString = malloc(size);
    if (!newString)
        return NULL;

    return memcpy(newString, str, size);
}

// From https://c-for-dummies.com/blog/?p=3863
int strcasecmp(const char *s1, const char *s2)
{
    int offset, ch;
    unsigned char a, b;

    offset = 0;
    ch = 0;
    while (*(s1 + offset) != '\0')
    {
        /* check for end of s2 */
        if (*(s2 + offset) == '\0')
            return(*(s1 + offset));

        a = (unsigned)*(s1 + offset);
        b = (unsigned)*(s2 + offset);
        ch = toupper(a) - toupper(b);
        if (ch<0 || ch>0)
            return(ch);
        offset++;
    }

    return(ch);
}

// modified from above
int strncasecmp(const char *s1, const char *s2, int len)
{
    //printf("strncasecmp(%s, %s, %d)\n", s1, s2, len);
    int offset, ch;
    unsigned char a, b;

    offset = 0;
    ch = 0;
    int i = 0;
    while (*(s1 + offset) != '\0' && i++ < len)
    {
        /* check for end of s2 */
        if (*(s2 + offset) == '\0')
            return(*(s1 + offset));

        a = (unsigned)*(s1 + offset);
        b = (unsigned)*(s2 + offset);
        ch = toupper(a) - toupper(b);
        if (ch<0 || ch>0)
            return(ch);
        offset++;
    }

    return(ch);
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

// TODO: Fix this and test it
int __cdecl strncmp(const char *str1, const char *str2, size_t len)
{    
    int dif = -1;
    do
    {
        dif = (int)*str1 - (int)*str2;
        if (dif)
            return dif;
        str1++;
        str2++;
    } while (*str1 != '\0' && --len > 0);

    return dif;
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

char * strrchr(const char *str, int ch)
{
    if (!str)
        return NULL;

    for (int i = strlen(str); i >= 0; --i)
    {
        if (str[i] == (char)ch)
            return &str[i];
    }

    return NULL;
}

int toupper(int arg)
{
    if (arg >= (int)'a' && arg <= (int)'z')
        arg += (int)'A' - (int)'a';

    return arg;
}

#ifdef DEBUG_MEM
void *dbg_malloc(size_t size, char *filename, int lineNumber)
{
    dbgMemFilename = filename;
    dbgMemLineNumber = lineNumber;
    serial_printf("Allocating %d bytes from %s, line %d for %s\n", size, filename, lineNumber, tasks[currentTask].imageName);
    return malloc(size);
}
#endif

unsigned int reuses = 0;
void* malloc(size_t size)
{
#ifdef MYOS_KERNEL
    return kmalloc(size);
#else
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
    // TODO: We have to guarantee that if we're calling this from the kernel, we don't try to reuse memory mapped to user space
    for (size_t i = 0; i < nextFreeMemorySlot; ++i)
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
            //printf("Reusing freed memory from slot %d, (reuse #%d)\n", i, ++reuses);
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

#endif // MYOS_KERNEL
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

// TODO: test
void * __cdecl memmove(void * destination, const void * source, size_t num)
{
    char *intermediate = malloc(num);

    if (!intermediate)
    {
        printf("failed to allocate %d bytes!\n", num);
        return NULL;
    }

    memcpy(intermediate, source, num);
    memcpy(destination, intermediate, num);

    free(intermediate);

    return destination;
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

#ifndef MYOS_KERNEL
// some code copied from printf.c that seems appropriate for user-space
// TODO: Don't duplicate code here

// internal flag definitions
#define FLAGS_ZEROPAD   (1U <<  0U)
#define FLAGS_LEFT      (1U <<  1U)
#define FLAGS_PLUS      (1U <<  2U)
#define FLAGS_SPACE     (1U <<  3U)
#define FLAGS_HASH      (1U <<  4U)
#define FLAGS_UPPERCASE (1U <<  5U)
#define FLAGS_CHAR      (1U <<  6U)
#define FLAGS_SHORT     (1U <<  7U)
#define FLAGS_LONG      (1U <<  8U)
#define FLAGS_LONG_LONG (1U <<  9U)
#define FLAGS_PRECISION (1U << 10U)
#define FLAGS_ADAPT_EXP (1U << 11U)

// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_NTOA_BUFFER_SIZE
#define PRINTF_NTOA_BUFFER_SIZE    32U
#endif

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_FTOA_BUFFER_SIZE
#define PRINTF_FTOA_BUFFER_SIZE    32U
#endif

// support for the floating point type (%f)
// default: activated
/*#ifndef PRINTF_DISABLE_SUPPORT_FLOAT
#define PRINTF_SUPPORT_FLOAT
#endif*/

// support for exponential floating point notation (%e/%g)
// default: activated
/*#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#define PRINTF_SUPPORT_EXPONENTIAL
#endif*/

// define the default floating point precision
// default: 6 digits
#ifndef PRINTF_DEFAULT_FLOAT_PRECISION
#define PRINTF_DEFAULT_FLOAT_PRECISION  6U
#endif

// define the largest float suitable to print with %f
// default: 1e9
#ifndef PRINTF_MAX_FLOAT
#define PRINTF_MAX_FLOAT  1e9
#endif

// support for the long long types (%llu or %p)
// default: activated
/*#ifndef PRINTF_DISABLE_SUPPORT_LONG_LONG
#define PRINTF_SUPPORT_LONG_LONG
#endif*/

// support for the ptrdiff_t type (%t)
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_PTRDIFF_T
#define PRINTF_SUPPORT_PTRDIFF_T
#endif

// output function type
typedef void(*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);

// internal buffer output
static inline void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen)
{
    if (idx < maxlen) {
        ((char*)buffer)[idx] = character;
    }
}

// internal null output
static inline void _out_null(char character, void* buffer, size_t idx, size_t maxlen)
{
    (void)character; (void)buffer; (void)idx; (void)maxlen;
}

// internal secure strlen
// \return The length of the string (excluding the terminating 0) limited by 'maxsize'
static inline unsigned int _strnlen_s(const char* str, size_t maxsize)
{
    const char* s;
    for (s = str; *s && maxsize--; ++s);
    return (unsigned int)(s - str);
}


// internal test if char is a digit (0-9)
// \return true if char is a digit
static inline bool _is_digit(char ch)
{
    return (ch >= '0') && (ch <= '9');
}


// internal ASCII string to unsigned int conversion
static unsigned int _atoi(const char** str)
{
    unsigned int i = 0U;
    while (_is_digit(**str)) {
        i = i * 10U + (unsigned int)(*((*str)++) - '0');
    }
    return i;
}

// output the specified string in reverse, taking care of any zero-padding
static size_t _out_rev(out_fct_type out, char* buffer, size_t idx, size_t maxlen, const char* buf, size_t len, unsigned int width, unsigned int flags)
{
    const size_t start_idx = idx;

    // pad spaces up to given width
    if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
        for (size_t i = len; i < width; i++) {
            out(' ', buffer, idx++, maxlen);
        }
    }

    // reverse string
    while (len) {
        out(buf[--len], buffer, idx++, maxlen);
    }

    // append pad spaces up to given width
    if (flags & FLAGS_LEFT) {
        while (idx - start_idx < width) {
            out(' ', buffer, idx++, maxlen);
        }
    }

    return idx;
}

// internal itoa format
static size_t _ntoa_format(out_fct_type out, char* buffer, size_t idx, size_t maxlen, char* buf, size_t len, bool negative, unsigned int base, unsigned int prec, unsigned int width, unsigned int flags)
{
    // pad leading zeros
    if (!(flags & FLAGS_LEFT)) {
        if (width && (flags & FLAGS_ZEROPAD) && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
            width--;
        }
        while ((len < prec) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = '0';
        }
        while ((flags & FLAGS_ZEROPAD) && (len < width) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = '0';
        }
    }

    // handle hash
    if (flags & FLAGS_HASH) {
        if (!(flags & FLAGS_PRECISION) && len && ((len == prec) || (len == width))) {
            len--;
            if (len && (base == 16U)) {
                len--;
            }
        }
        if ((base == 16U) && !(flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = 'x';
        }
        else if ((base == 16U) && (flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = 'X';
        }
        else if ((base == 2U) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = 'b';
        }
        if (len < PRINTF_NTOA_BUFFER_SIZE) {
            buf[len++] = '0';
        }
    }

    if (len < PRINTF_NTOA_BUFFER_SIZE) {
        if (negative) {
            buf[len++] = '-';
        }
        else if (flags & FLAGS_PLUS) {
            buf[len++] = '+';  // ignore the space if the '+' exists
        }
        else if (flags & FLAGS_SPACE) {
            buf[len++] = ' ';
        }
    }

    return _out_rev(out, buffer, idx, maxlen, buf, len, width, flags);
}

// internal itoa for 'long long' type
#if defined(PRINTF_SUPPORT_LONG_LONG)
static size_t _ntoa_long_long(out_fct_type out, char* buffer, size_t idx, size_t maxlen, unsigned long long value, bool negative, unsigned long long base, unsigned int prec, unsigned int width, unsigned int flags)
{
    char buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0U;

    // no hash for 0 values
    if (!value) {
        flags &= ~FLAGS_HASH;
    }

    // write if precision != 0 and value is != 0
    if (!(flags & FLAGS_PRECISION) || value) {
        do {
            const char digit = (char)(value % base);
            buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
            value /= base;
        } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }

    return _ntoa_format(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width, flags);
}
#endif  // PRINTF_SUPPORT_LONG_LONG


#if defined(PRINTF_SUPPORT_FLOAT)

#if defined(PRINTF_SUPPORT_EXPONENTIAL)
// forward declaration so that _ftoa can switch to exp notation for values > PRINTF_MAX_FLOAT
static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags);
#endif


// internal ftoa for fixed decimal floating point
static size_t _ftoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags)
{
    char buf[PRINTF_FTOA_BUFFER_SIZE];
    size_t len = 0U;
    double diff = 0.0;

    // powers of 10
    static const double pow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

    // test for special values
    if (value != value)
        return _out_rev(out, buffer, idx, maxlen, "nan", 3, width, flags);
    if (value < -DBL_MAX)
        return _out_rev(out, buffer, idx, maxlen, "fni-", 4, width, flags);
    if (value > DBL_MAX)
        return _out_rev(out, buffer, idx, maxlen, (flags & FLAGS_PLUS) ? "fni+" : "fni", (flags & FLAGS_PLUS) ? 4U : 3U, width, flags);

    // test for very large values
    // standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters overflowing your buffers == bad
    if ((value > PRINTF_MAX_FLOAT) || (value < -PRINTF_MAX_FLOAT)) {
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
        return _etoa(out, buffer, idx, maxlen, value, prec, width, flags);
#else
        return 0U;
#endif
    }

    // test for negative
    bool negative = false;
    if (value < 0) {
        negative = true;
        value = 0 - value;
    }

    // set default precision, if not set explicitly
    if (!(flags & FLAGS_PRECISION)) {
        prec = PRINTF_DEFAULT_FLOAT_PRECISION;
    }
    // limit precision to 9, cause a prec >= 10 can lead to overflow errors
    while ((len < PRINTF_FTOA_BUFFER_SIZE) && (prec > 9U)) {
        buf[len++] = '0';
        prec--;
    }

    int whole = (int)value;
    double tmp = (value - whole) * pow10[prec];
    unsigned long frac = (unsigned long)tmp;
    diff = tmp - frac;

    if (diff > 0.5) {
        ++frac;
        // handle rollover, e.g. case 0.99 with prec 1 is 1.0
        if (frac >= pow10[prec]) {
            frac = 0;
            ++whole;
        }
    }
    else if (diff < 0.5) {
    }
    else if ((frac == 0U) || (frac & 1U)) {
        // if halfway, round up if odd OR if last digit is 0
        ++frac;
    }

    if (prec == 0U) {
        diff = value - (double)whole;
        if ((!(diff < 0.5) || (diff > 0.5)) && (whole & 1)) {
            // exactly 0.5 and ODD, then round up
            // 1.5 -> 2, but 2.5 -> 2
            ++whole;
        }
    }
    else {
        unsigned int count = prec;
        // now do fractional part, as an unsigned number
        while (len < PRINTF_FTOA_BUFFER_SIZE) {
            --count;
            buf[len++] = (char)(48U + (frac % 10U));
            if (!(frac /= 10U)) {
                break;
            }
        }
        // add extra 0s
        while ((len < PRINTF_FTOA_BUFFER_SIZE) && (count-- > 0U)) {
            buf[len++] = '0';
        }
        if (len < PRINTF_FTOA_BUFFER_SIZE) {
            // add decimal
            buf[len++] = '.';
        }
    }

    // do whole part, number is reversed
    while (len < PRINTF_FTOA_BUFFER_SIZE) {
        buf[len++] = (char)(48 + (whole % 10));
        if (!(whole /= 10)) {
            break;
        }
    }

    // pad leading zeros
    if (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD)) {
        if (width && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
            width--;
        }
        while ((len < width) && (len < PRINTF_FTOA_BUFFER_SIZE)) {
            buf[len++] = '0';
        }
    }

    if (len < PRINTF_FTOA_BUFFER_SIZE) {
        if (negative) {
            buf[len++] = '-';
        }
        else if (flags & FLAGS_PLUS) {
            buf[len++] = '+';  // ignore the space if the '+' exists
        }
        else if (flags & FLAGS_SPACE) {
            buf[len++] = ' ';
        }
    }

    return _out_rev(out, buffer, idx, maxlen, buf, len, width, flags);
}

static size_t _ntoa_long(out_fct_type out, char* buffer, size_t idx, size_t maxlen, unsigned long value, bool negative, unsigned long base, unsigned int prec, unsigned int width, unsigned int flags);

#if defined(PRINTF_SUPPORT_EXPONENTIAL)
// internal ftoa variant for exponential floating-point type, contributed by Martijn Jasperse <m.jasperse@gmail.com>
static size_t _etoa(out_fct_type out, char* buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags)
{
    // check for NaN and special values
    if ((value != value) || (value > DBL_MAX) || (value < -DBL_MAX)) {
        return _ftoa(out, buffer, idx, maxlen, value, prec, width, flags);
    }

    // determine the sign
    const bool negative = value < 0;
    if (negative) {
        value = -value;
    }

    // default precision
    if (!(flags & FLAGS_PRECISION)) {
        prec = PRINTF_DEFAULT_FLOAT_PRECISION;
    }

    // determine the decimal exponent
    // based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
    union {
        uint64_t U;
        double   F;
    } conv;

    conv.F = value;
    int exp2 = (int)((conv.U >> 52U) & 0x07FFU) - 1023;           // effectively log2
    conv.U = (conv.U & ((1ULL << 52U) - 1U)) | (1023ULL << 52U);  // drop the exponent so conv.F is now in [1,2)
                                                                  // now approximate log10 from the log2 integer part and an expansion of ln around 1.5
    int expval = (int)(0.1760912590558 + exp2 * 0.301029995663981 + (conv.F - 1.5) * 0.289529654602168);
    // now we want to compute 10^expval but we want to be sure it won't overflow
    exp2 = (int)(expval * 3.321928094887362 + 0.5);
    const double z = expval * 2.302585092994046 - exp2 * 0.6931471805599453;
    const double z2 = z * z;
    conv.U = (uint64_t)(exp2 + 1023) << 52U;
    // compute exp(z) using continued fractions, see https://en.wikipedia.org/wiki/Exponential_function#Continued_fractions_for_ex
    conv.F *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));
    // correct for rounding errors
    if (value < conv.F) {
        expval--;
        conv.F /= 10;
    }

    // the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters
    unsigned int minwidth = ((expval < 100) && (expval > -100)) ? 4U : 5U;

    // in "%g" mode, "prec" is the number of *significant figures* not decimals
    if (flags & FLAGS_ADAPT_EXP) {
        // do we want to fall-back to "%f" mode?
        if ((value >= 1e-4) && (value < 1e6)) {
            if ((int)prec > expval) {
                prec = (unsigned)((int)prec - expval - 1);
            }
            else {
                prec = 0;
            }
            flags |= FLAGS_PRECISION;   // make sure _ftoa respects precision
                                        // no characters in exponent
            minwidth = 0U;
            expval = 0;
        }
        else {
            // we use one sigfig for the whole part
            if ((prec > 0) && (flags & FLAGS_PRECISION)) {
                --prec;
            }
        }
    }

    // will everything fit?
    unsigned int fwidth = width;
    if (width > minwidth) {
        // we didn't fall-back so subtract the characters required for the exponent
        fwidth -= minwidth;
    }
    else {
        // not enough characters, so go back to default sizing
        fwidth = 0U;
    }
    if ((flags & FLAGS_LEFT) && minwidth) {
        // if we're padding on the right, DON'T pad the floating part
        fwidth = 0U;
    }

    // rescale the float value
    if (expval) {
        value /= conv.F;
    }

    // output the floating part
    const size_t start_idx = idx;
    idx = _ftoa(out, buffer, idx, maxlen, negative ? -value : value, prec, fwidth, flags & ~FLAGS_ADAPT_EXP);

    // output the exponent part
    if (minwidth) {
        // output the exponential symbol
        out((flags & FLAGS_UPPERCASE) ? 'E' : 'e', buffer, idx++, maxlen);
        // output the exponent value
        idx = _ntoa_long(out, buffer, idx, maxlen, (expval < 0) ? -expval : expval, expval < 0, 10, 0, minwidth - 1, FLAGS_ZEROPAD | FLAGS_PLUS);
        // might need to right-pad spaces
        if (flags & FLAGS_LEFT) {
            while (idx - start_idx < width) out(' ', buffer, idx++, maxlen);
        }
    }
    return idx;
}
#endif  // PRINTF_SUPPORT_EXPONENTIAL
#endif  // PRINTF_SUPPORT_FLOAT


// internal itoa for 'long' type
static size_t _ntoa_long(out_fct_type out, char* buffer, size_t idx, size_t maxlen, unsigned long value, bool negative, unsigned long base, unsigned int prec, unsigned int width, unsigned int flags)
{
    char buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0U;

    // no hash for 0 values
    if (!value) {
        flags &= ~FLAGS_HASH;
    }

    // write if precision != 0 and value is != 0
    if (!(flags & FLAGS_PRECISION) || value) {
        do {
            const char digit = (char)(value % base);
            buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
            value /= base;
        } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }

    return _ntoa_format(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width, flags);
}

static int _vsnprintf(out_fct_type out, char* buffer, const size_t maxlen, const char* format, va_list va)
{
    unsigned int flags, width, precision, n;
    size_t idx = 0U;

    if (!buffer) {
        // use null output function
        out = _out_null;
    }

    while (*format)
    {
        // format specifier?  %[flags][width][.precision][length]
        if (*format != '%') {
            // no
            out(*format, buffer, idx++, maxlen);
            format++;
            continue;
        }
        else {
            // yes, evaluate it
            format++;
        }

        // evaluate flags
        flags = 0U;
        do {
            switch (*format) {
                case '0': flags |= FLAGS_ZEROPAD; format++; n = 1U; break;
                case '-': flags |= FLAGS_LEFT;    format++; n = 1U; break;
                case '+': flags |= FLAGS_PLUS;    format++; n = 1U; break;
                case ' ': flags |= FLAGS_SPACE;   format++; n = 1U; break;
                case '#': flags |= FLAGS_HASH;    format++; n = 1U; break;
                default:                                   n = 0U; break;
            }
        } while (n);

        // evaluate width field
        width = 0U;
        if (_is_digit(*format)) {
            width = _atoi(&format);
        }
        else if (*format == '*') {
            const int w = va_arg(va, int);
            if (w < 0) {
                flags |= FLAGS_LEFT;    // reverse padding
                width = (unsigned int)-w;
            }
            else {
                width = (unsigned int)w;
            }
            format++;
        }

        // evaluate precision field
        precision = 0U;
        if (*format == '.') {
            flags |= FLAGS_PRECISION;
            format++;
            if (_is_digit(*format)) {
                precision = _atoi(&format);
            }
            else if (*format == '*') {
                const int prec = (int)va_arg(va, int);
                precision = prec > 0 ? (unsigned int)prec : 0U;
                format++;
            }
        }

        // evaluate length field
        switch (*format) {
            case 'l':
                flags |= FLAGS_LONG;
                format++;
                if (*format == 'l') {
                    flags |= FLAGS_LONG_LONG;
                    format++;
                }
                break;
            case 'h':
                flags |= FLAGS_SHORT;
                format++;
                if (*format == 'h') {
                    flags |= FLAGS_CHAR;
                    format++;
                }
                break;
#if defined(PRINTF_SUPPORT_PTRDIFF_T)
            case 't':
                flags |= (sizeof(ptrdiff_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
                format++;
                break;
#endif
            case 'j':
                flags |= (sizeof(intmax_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
                format++;
                break;
            case 'z':
                flags |= (sizeof(size_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
                format++;
                break;
            default:
                break;
        }

        // evaluate specifier
        switch (*format) {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'o':
            case 'b': {
                // set the base
                unsigned int base;
                if (*format == 'x' || *format == 'X') {
                    base = 16U;
                }
                else if (*format == 'o') {
                    base = 8U;
                }
                else if (*format == 'b') {
                    base = 2U;
                }
                else {
                    base = 10U;
                    flags &= ~FLAGS_HASH;   // no hash for dec format
                }
                // uppercase
                if (*format == 'X') {
                    flags |= FLAGS_UPPERCASE;
                }

                // no plus or space flag for u, x, X, o, b
                if ((*format != 'i') && (*format != 'd')) {
                    flags &= ~(FLAGS_PLUS | FLAGS_SPACE);
                }

                // ignore '0' flag when precision is given
                if (flags & FLAGS_PRECISION) {
                    flags &= ~FLAGS_ZEROPAD;
                }

                // convert the integer
                if ((*format == 'i') || (*format == 'd')) {
                    // signed
                    if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_SUPPORT_LONG_LONG)
                        const long long value = va_arg(va, long long);
                        idx = _ntoa_long_long(out, buffer, idx, maxlen, (unsigned long long)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
#endif
                    }
                    else if (flags & FLAGS_LONG) {
                        const long value = va_arg(va, long);
                        idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
                    }
                    else {
                        const int value = (flags & FLAGS_CHAR) ? (char)va_arg(va, int) : (flags & FLAGS_SHORT) ? (short int)va_arg(va, int) : va_arg(va, int);
                        idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned int)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
                    }
                }
                else {
                    // unsigned
                    if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_SUPPORT_LONG_LONG)
                        idx = _ntoa_long_long(out, buffer, idx, maxlen, va_arg(va, unsigned long long), false, base, precision, width, flags);
#endif
                    }
                    else if (flags & FLAGS_LONG) {
                        idx = _ntoa_long(out, buffer, idx, maxlen, va_arg(va, unsigned long), false, base, precision, width, flags);
                    }
                    else {
                        const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char)va_arg(va, unsigned int) : (flags & FLAGS_SHORT) ? (unsigned short int)va_arg(va, unsigned int) : va_arg(va, unsigned int);
                        idx = _ntoa_long(out, buffer, idx, maxlen, value, false, base, precision, width, flags);
                    }
                }
                format++;
                break;
            }
#if defined(PRINTF_SUPPORT_FLOAT)
            case 'f':
            case 'F':
                if (*format == 'F') flags |= FLAGS_UPPERCASE;
                idx = _ftoa(out, buffer, idx, maxlen, va_arg(va, double), precision, width, flags);
                format++;
                break;
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                if ((*format == 'g') || (*format == 'G')) flags |= FLAGS_ADAPT_EXP;
                if ((*format == 'E') || (*format == 'G')) flags |= FLAGS_UPPERCASE;
                idx = _etoa(out, buffer, idx, maxlen, va_arg(va, double), precision, width, flags);
                format++;
                break;
#endif  // PRINTF_SUPPORT_EXPONENTIAL
#endif  // PRINTF_SUPPORT_FLOAT
            case 'c': {
                unsigned int l = 1U;
                // pre padding
                if (!(flags & FLAGS_LEFT)) {
                    while (l++ < width) {
                        out(' ', buffer, idx++, maxlen);
                    }
                }
                // char output
                out((char)va_arg(va, int), buffer, idx++, maxlen);
                // post padding
                if (flags & FLAGS_LEFT) {
                    while (l++ < width) {
                        out(' ', buffer, idx++, maxlen);
                    }
                }
                format++;
                break;
            }

            case 's': {
                const char* p = va_arg(va, char*);
                unsigned int l = _strnlen_s(p, precision ? precision : (size_t)-1);
                // pre padding
                if (flags & FLAGS_PRECISION) {
                    l = (l < precision ? l : precision);
                }
                if (!(flags & FLAGS_LEFT)) {
                    while (l++ < width) {
                        out(' ', buffer, idx++, maxlen);
                    }
                }
                // string output
                while ((*p != 0) && (!(flags & FLAGS_PRECISION) || precision--)) {
                    out(*(p++), buffer, idx++, maxlen);
                }
                // post padding
                if (flags & FLAGS_LEFT) {
                    while (l++ < width) {
                        out(' ', buffer, idx++, maxlen);
                    }
                }
                format++;
                break;
            }

            case 'p': {
                width = sizeof(void*) * 2U;
                flags |= FLAGS_ZEROPAD | FLAGS_UPPERCASE;
#if defined(PRINTF_SUPPORT_LONG_LONG)
                const bool is_ll = sizeof(uintptr_t) == sizeof(long long);
                if (is_ll) {
                    idx = _ntoa_long_long(out, buffer, idx, maxlen, (uintptr_t)va_arg(va, void*), false, 16U, precision, width, flags);
                }
                else {
#endif
                    idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long)((uintptr_t)va_arg(va, void*)), false, 16U, precision, width, flags);
#if defined(PRINTF_SUPPORT_LONG_LONG)
                }
#endif
                format++;
                break;
            }

            case '%':
                out('%', buffer, idx++, maxlen);
                format++;
                break;

            default:
                out(*format, buffer, idx++, maxlen);
                format++;
                break;
        }
    }

    // termination
    out((char)0, buffer, idx < maxlen ? idx : maxlen - 1U, maxlen);

    // return written chars without terminating \0
    return (int)idx;
}

int vsnprintf_(char* buffer, size_t count, const char* format, va_list va)
{
    return _vsnprintf(_out_buffer, buffer, count, format, va);
}

int snprintf_(char* buffer, size_t count, const char* format, ...)
{
    va_list va;
    va_start(va, format);
    const int ret = _vsnprintf(_out_buffer, buffer, count, format, va);
    va_end(va);
    return ret;
}
#endif
