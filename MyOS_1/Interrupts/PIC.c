#include "Interrupts.h"
#include "PIC.h"
#include "../System_Specific.h"
#include <stdint.h>

void IRQ_Enable_Line(unsigned char IRQline)
{
    uint16_t port;
    uint8_t value;

    if (IRQline < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        IRQline -= 8;

        // Make sure IRQ2 of PIC1 is enabled or we'll never receive the interrupt from PIC2
        value = inb(PIC1_DATA) & ~(1 << 2);
        outb(PIC1_DATA, value);
        io_wait();
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);
}

void IRQ_Disable_Line(unsigned char IRQline) 
{
    uint16_t port;
    uint8_t value;

    if (IRQline < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);
}

void PIC_sendEOI(unsigned char irq)
{
    if (irq >= 8 + HARDWARE_INTERRUPTS_BASE)
        outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}

/*
Reinitialize the PIC controllers, giving them specified vector offsets
rather than 8h and 70h, as configured by default
arguments:
offset1 - vector offset for master PIC
vectors on the master become offset1..offset1+7
offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(unsigned char offset1, unsigned char offset2)
{
    unsigned char a1, a2;

    a1 = inb(PIC1_DATA);                        // save masks
    a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb((unsigned char)PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
    io_wait();
    outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
    io_wait();
    outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    io_wait();
    outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, a1);   // restore saved masks.
    outb(PIC2_DATA, a2);
}

/* Helper func */
inline uint16_t pic_get_irq_reg(unsigned char ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
    * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(void)
{
    return pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(void)
{
    return pic_get_irq_reg(PIC_READ_ISR);
}