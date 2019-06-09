#pragma once

#include <stdbool.h>

#define SYSCALL_PRINT               255
#define SYSCALL_PRINTF              254
#define SYSCALL_GET_GRAPHICS_INFO   253

void SystemCallGetGraphicsInfo(bool *graphicsArePresent, int *width, int *height);
#define getGraphicsInfo SystemCallGetGraphicsInfo

void SystemCallPrint(char *str);

#define printf SystemCallPrintf
int __cdecl SystemCallPrintf(const char* format, ...);
