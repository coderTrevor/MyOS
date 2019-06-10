#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "../Graphics/Display_HAL.h"
#include "../Libs/SDL/include/SDL_rect.h"

#define SYSCALL_PRINT               255
#define SYSCALL_PRINTF              254
#define SYSCALL_GET_GRAPHICS_INFO   253
#define SYSCALL_PAGE_ALLOCATOR      252
#define SYSCALL_TIME_DELAY_MS       251
#define SYSCALL_GRAPHICS_BLIT       250

void SystemCallPageAllocator(unsigned int pages, unsigned int *pPagesAllocated, uint32_t *pRreturnVal);
#define pageAllocator SystemCallPageAllocator

void SystemCallGetGraphicsInfo(bool *graphicsArePresent, int *width, int *height);
#define getGraphicsInfo SystemCallGetGraphicsInfo

void SystemCallGraphicsBlit(const SDL_Rect *sourceRect, PIXEL_32BIT *image);
#define graphicsBlit SystemCallGraphicsBlit

void SystemCallPrint(char *str);

#define printf SystemCallPrintf
int __cdecl SystemCallPrintf(const char* format, ...);

void SystemCallTimeDelayMS(uint32_t milliSeconds);
#define timeDelayMS SystemCallTimeDelayMS