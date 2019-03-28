#include "GDT.h"

/* Our GDT, with 3 entries, and finally our special GDT pointer */
GDT_Descriptor globalDescriptorTable[3];
GDT_ptr_struct gdt_ptr_struct;

void GDT_Init()
{ 
    /* Setup the GDT pointer and limit */
    gdt_ptr_struct.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gdt_ptr_struct.base = (uint32_t)&globalDescriptorTable;

    /* Our NULL descriptor */
    gdt_set_gate(&globalDescriptorTable[0], 0, 0, 0, 0);

    /* The second entry is our Code Segment. The base address
    *  is 0, the limit is 4GBytes, it uses 4KByte granularity,
    *  uses 32-bit opcodes, and is a Code Segment descriptor. */
    gdt_set_gate(&globalDescriptorTable[1], 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* The third entry is our Data Segment. It's EXACTLY the
    *  same as our code segment, but the descriptor type in
    *  this entry's access byte says it's a Data Segment */
    gdt_set_gate(&globalDescriptorTable[2], 0, 0xFFFFFFFF, 0x92, 0xCF);

    // load the gdt we just setup
    _asm
    {
        lgdt gdt_ptr_struct
    }

    // update the segments to use the new gdt
    flush_GDT();
}

inline void gdt_set_gate(GDT_Descriptor *gdtEntry, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    /* Setup the descriptor base address */
    gdtEntry->base_low = (base & 0xFFFF);
    gdtEntry->base_middle = (base >> 16) & 0xFF;
    gdtEntry->base_high = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdtEntry->limit_low = (limit & 0xFFFF);
    gdtEntry->granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdtEntry->granularity |= (gran & 0xF0);
    gdtEntry->access = access;
}

inline void flush_GDT()
{
    _asm
    {
        push eax

        mov eax, 0x08
        push eax
        mov eax, [reload]
        push eax
        retf
        reload :

        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        pop eax
    }
}