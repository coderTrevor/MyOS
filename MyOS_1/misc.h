#pragma once
#include <stdint.h>
#include <stdbool.h>

extern int debugLevel;
extern bool showOverlay;

char intToChar(int i);

size_t __cdecl strlen(const char* str);
#pragma intrinsic(strlen)

int __cdecl strcmp(const char *str1, const char *str2);
#pragma intrinsic(strcmp)

int __cdecl memcmp(const void *ptr1, const void *ptr2, size_t num);
#pragma intrinsic(memcmp)

char * __cdecl strncpy(char * destination, const char * source, size_t num);

void * __cdecl memcpy(void* dest, const void* src, size_t count);
#pragma intrinsic(memcpy)

void * __cdecl memset(void *ptr, int value, size_t num);
#pragma intrinsic(memset)
