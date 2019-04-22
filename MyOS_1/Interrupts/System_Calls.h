#pragma once

#define SYSCALL_PRINT    255
#define SYSCALL_PRINTF   254

void SystemCallPrint(char *str);

#define printf SystemCallPrintf
int SystemCallPrintf(const char* format, ...);
