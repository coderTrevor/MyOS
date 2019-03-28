#pragma once
#include <stdint.h>

#pragma pack(push, 1)
typedef struct IDT_ENTRY
{
    uint16_t offset_lowerbits;  // offset bits 0..15
    uint16_t selector;  // a code segment selector in GDT or LDT
    uint8_t  zero;      // unused, set to 0
    uint8_t  type_attr; // type and attributes, see below
    uint16_t offset_higherbits;  // offset bits 16..31
} IDT_ENTRY;

typedef struct IDT_ptr_struct
{
    uint16_t size;
    uint32_t base;                // The address of the first IDT_ENTRY struct.
} IDT_ptr_struct;

#pragma pack(pop, 1)

extern IDT_ENTRY IDT[256];

void IDT_Init(void);

void Set_IDT_Entry(unsigned long IRQ_Handler, int IDT_Number);

