#pragma once
#include <stdint.h>
#include <stdbool.h>

//#define DEBUG_MEM

#ifdef DEBUG_MEM
#define MAX_DEBUG_FILENAME_LENGTH 24
typedef char MEM_FILENAME[MAX_DEBUG_FILENAME_LENGTH];
extern char *dbgMemFilename;
extern int   dbgMemLineNumber;
extern char *dbgFreeFilename;
extern int   dbgFreeLineNumber;
#endif 

#define MAX_ALLOCATIONS   4096
// Struct-of-arrays for memory allocation
// TODO: Make more dynamic
// TEMPTEMP: This is super-inefficient and mostly just a placeholder until something better is required
typedef struct ALLOCATION_ARRAY
{
    uint32_t        address[MAX_ALLOCATIONS];
    uint32_t        size[MAX_ALLOCATIONS];
    bool            inUse[MAX_ALLOCATIONS];
#ifdef DEBUG_MEM
    MEM_FILENAME    filename[MAX_ALLOCATIONS];
    unsigned int    lineNumber[MAX_ALLOCATIONS];
#endif
} ALLOCATION_ARRAY;

extern ALLOCATION_ARRAY allocationArray;
extern unsigned int nextAllocationSlot;

extern ALLOCATION_ARRAY freeMemoryArray;
extern unsigned int nextFreeMemorySlot;

extern int debugLevel;
extern bool showOverlay;

void* calloc(size_t num, size_t size);

void *dbg_malloc(size_t size, char *filename, int lineNumber);

void dbg_free(void *ptr, char *filename, int lineNumber);

void _exit(int status);

void free(void *ptr);

char intToChar(int i);

size_t __cdecl strlen(const char* str);
#pragma intrinsic(strlen)

int __cdecl strcmp(const char *str1, const char *str2);
#pragma intrinsic(strcmp)

#ifdef DEBUG_MEM
// TODO: Not thread safe, but it probably doesn't matter
#define dbg_alloc(sz)    dbg_malloc(sz, __FILE__, __LINE__)
#define dbg_release(ptr) dbg_free(ptr,  __FILE__, __LINE__)
#else
#define dbg_alloc(sz)      malloc(sz)
#define dbg_release(ptr)   free(ptr)
#endif

void *malloc(size_t size);

int __cdecl memcmp(const void *ptr1, const void *ptr2, size_t num);
#pragma intrinsic(memcmp)

void* realloc(void* ptr, size_t size);

char * __cdecl strchr(char *str, int character);

char * __cdecl strncpy(char * destination, const char * source, size_t num);

void * __cdecl memcpy(void* dest, const void* src, size_t count);
#pragma intrinsic(memcpy)

void * __cdecl memset(void *ptr, int value, size_t num);
#pragma intrinsic(memset)
