#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SYSCALL_PRINT               255
#define SYSCALL_PRINTF              254
#define SYSCALL_GET_GRAPHICS_INFO   253
#define SYSCALL_PAGE_ALLOCATOR      252

void SystemCallPageAllocator(unsigned int pages, unsigned int *pPagesAllocated, uint32_t *pRreturnVal);
#define pageAllocator SystemCallPageAllocator

void SystemCallGetGraphicsInfo(bool *graphicsArePresent, int *width, int *height);
#define getGraphicsInfo SystemCallGetGraphicsInfo

void SystemCallPrint(char *str);

#define printf SystemCallPrintf
int __cdecl SystemCallPrintf(const char* format, ...);
