#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MAX_ALLOCATIONS   2048
// Struct-of-arrays for memory allocation
// TODO: Make more dynamic
// TEMPTEMP: This is super-inefficient and mostly just a placeholder until something better is required
// NOTE: At this point, freed memory is never reclaimed
typedef struct ALLOCATION_ARRAY
{
    uint32_t    address[MAX_ALLOCATIONS];
    uint32_t    size[MAX_ALLOCATIONS];
    bool        inUse[MAX_ALLOCATIONS];
} ALLOCATION_ARRAY;

extern ALLOCATION_ARRAY allocationArray;
extern unsigned int nextAllocationSlot;

extern int debugLevel;
extern bool showOverlay;

// TODO: Fix MSVC crying about redefinition
//void free(void *ptr);

char intToChar(int i);

size_t __cdecl strlen(const char* str);
#pragma intrinsic(strlen)

int __cdecl strcmp(const char *str1, const char *str2);
#pragma intrinsic(strcmp)

// TODO: Fix MSVC crying about redefinition
//void* malloc(size_t size);

int __cdecl memcmp(const void *ptr1, const void *ptr2, size_t num);
#pragma intrinsic(memcmp)

char * __cdecl strchr(char *str, int character);

char * __cdecl strncpy(char * destination, const char * source, size_t num);

void * __cdecl memcpy(void* dest, const void* src, size_t count);
#pragma intrinsic(memcpy)

void * __cdecl memset(void *ptr, int value, size_t num);
#pragma intrinsic(memset)
