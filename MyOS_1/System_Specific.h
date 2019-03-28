#pragma once
#include <intrin.h>
#include <stdint.h>

// Code that's (very) specific to a given compiler / host environment

inline unsigned char inb(unsigned short Port)
{
    return __inbyte(Port);
}

inline uint16_t inw(unsigned short Port)
{
    return __inword(Port);
}

inline unsigned long inl(unsigned short Port)
{
    return __indword(Port);
}

static inline void io_wait(void)
{
    /* Port 0x80 is used for 'checkpoints' during POST. */
    /* The Linux kernel seems to think it is free for use :-/ */
    __outbyte(0x80, 0);
    //asm volatile ("outb %%al, $0x80" : : "a"(0));
    /* %%al instead of %0 makes no difference.  TODO: does the register need to be zeroed? */
}

inline void outb(unsigned short Port, unsigned char Data)
{
    __outbyte(Port, Data);
}

inline void outl(unsigned short Port, unsigned long Data)
{
    __outdword(Port, Data);
}

inline void outw(unsigned short Port, uint16_t Data)
{
    __outword(Port, Data);
}
