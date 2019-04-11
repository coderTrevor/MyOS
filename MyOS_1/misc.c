#include <stdint.h>
#include "misc.h"

char intToChar(int i)
{
    char *ints = "0123456789";
    if( i > 9 || i < 0)
        return '?';
    return ints[i];
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