#include "../Drivers/Keyboard.h"
#include "../Drivers/RTL_8139.h"
#include "IDT.h"
#include "Interrupts.h"
#include <intrin.h>

IDT_ENTRY IDT[256];
IDT_ptr_struct IDT_ptr;

void Set_IDT_Entry(unsigned long IRQ_Handler, int IDT_Number)
{
    IDT[IDT_Number].offset_lowerbits = IRQ_Handler & 0xffff;
    IDT[IDT_Number].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    IDT[IDT_Number].zero = 0;
    IDT[IDT_Number].type_attr = 0x8e; /* INTERRUPT_GATE */
    IDT[IDT_Number].offset_higherbits = (IRQ_Handler & 0xffff0000) >> 16;
}

void IDT_Init(void)
{
    interrupts_fired = 0;

    // set default exception handlers
    for (int i = 0; i < 32; ++i)
    {
        Set_IDT_Entry((unsigned long)default_exception_handler, i);
    }

    // set default handlers
    for (int i = 32; i < 256; ++i)
    {
        Set_IDT_Entry((unsigned long)default_interrupt_handler, i);
    }

    // set keyboard handler (interrupt 1 is remapped to 33, 0x21)
    Set_IDT_Entry((unsigned long)keyboard_interrupt_handler, HARDWARE_INTERRUPTS_BASE + 1);

    /* fill the IDT descriptor */
    IDT_ptr.base = (uint32_t)IDT;
    IDT_ptr.size = (sizeof(IDT_ENTRY) * 256);
    /*idt_address = (unsigned long)IDT;
    idt_ptr[0] = (sizeof(IDT_ENTRY) * 256) + ((idt_address & 0xffff) << 16);
    idt_ptr[1] = idt_address >> 16;*/

    // load the idt pointer and enable interrupts
    _asm
    {
        lidt IDT_ptr
        sti
    }
}