#pragma once
#include <stdint.h>

#define GRANULARITY_4KiB    (1 << 23)
#define SIZE_32BIT          (1 << 22)
#define CODE_ACCESS         0x9A
#define DATA_ACCESS         0x92
//                          base 24:31 - Flags (Gr,Sz,0,0) - Limit 16:19 - Acces (Pr, Privl 00, S, Ex, DC, RW, Ac) - Base 16:23 - Base 0:15 - Limit 0:15
//                          0x00            C                   F           9A                                           00         0000        FFFF
#define CODE_GDT_ENTRY      0x00CF9A000000FFFF
#define DATA_GDT_ENTRY      0x00CF92000000FFFF

/*#define CODE_GDT_ENTRY_UPPER      0x00CF9A00
#define CODE_GDT_ENTRY_LOWER      0x0000FFFF
#define DATA_GDT_ENTRY_UPPER      0x00CF9200
#define DATA_GDT_ENTRY_LOWER      0x0000FFFF*/
/*#define CODE_GDT_ENTRY_UPPER      0xFFFF0000
#define CODE_GDT_ENTRY_LOWER      0x009ACF00
#define DATA_GDT_ENTRY_UPPER      0xFFFF0000
#define DATA_GDT_ENTRY_LOWER      0x0092CF00*/

#pragma pack(push, 1)
typedef struct gdt_ptr_struct
{
    uint16_t limit;               // The upper 16 bits of all selector limits.
    uint32_t base;                // The address of the first gdt_entry_t struct.
} GDT_ptr_struct, gdt_ptr;

typedef struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} gdt_entry, GDT_Descriptor;

#pragma pack(pop, 1)


extern GDT_ptr_struct gdt_ptr_struct;
extern GDT_Descriptor globalDescriptorTable[3];

void GDT_Init();

void flush_GDT();

void gdt_set_gate(GDT_Descriptor *gdt, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
