// Driver file for Adlib sound card / OPL2 synth

#include "AdLib.h"
#include "../Terminal.h"
#include "../misc.h"
#include "../System_Specific.h"
#include "../Timers/System_Clock.h"

uint16_t Adlib_BaseAddress = 0x388;
uint16_t Adlib_DataPort = 0x389;

uint8_t Adlib_IRQ = 8;
bool Adlib_Present = false;

bool Adlib_Init(void)
{
    // Try to find an Adlib card
    terminal_writestring("Searching for an Adlib compatible sound device...\n");

    // Reset both timers
    Adlib_Write(REG_TIMER_CONTROL, TIMER_CTRL_TIMER1_MASK | TIMER_CTRL_TIMER2_MASK); // Not sure why this first step is needed
    Adlib_Write(REG_TIMER_CONTROL, TIMER_CTRL_IRQ_RESET);

    uint8_t status = Adlib_Read(REG_STATUS);

    // Write 0xff to timer 1 register
    Adlib_Write(REG_TIMER1_VALUE, 0xFF);

    // Start Timer 1
    Adlib_Write(REG_TIMER_CONTROL, TIMER_CTRL_TIMER2_MASK | TIMER_CTRL_TIMER1_GO);

    // Delay for at least 80 microseconds (already done in Adlib_Write)
    TimeDelayMS(1);

    // Read the status register again
    uint8_t status2 = Adlib_Read(REG_STATUS);

    // Reset both timers and interrupts (not sure why but the instructions say to do this)
    Adlib_Write(REG_TIMER_CONTROL, TIMER_CTRL_TIMER1_MASK | TIMER_CTRL_TIMER2_MASK); // Still not sure why this is needed, if indeed it is
    Adlib_Write(REG_TIMER_CONTROL, TIMER_CTRL_IRQ_RESET);

    // Test the stored status
    if ((status & 0xE0) == 0)
    {
        // test the second stored status
        if ((status2 & 0xE0) == 0xC0)
        {
            terminal_writestring("Found an Adlib compatible device at ");
            terminal_print_ushort_hex(Adlib_BaseAddress);
            terminal_newline();
            Adlib_Present = true;
        }
    }

    if (!Adlib_Present)
        terminal_writestring("No Adlib compatible device found\n");

    return Adlib_Present;
}

uint8_t Adlib_Read(uint8_t reg)
{
    return inb(Adlib_BaseAddress + reg);
}

void Adlib_Reset(void)
{
    terminal_writestring("Resetting Adlib...\n");

    // Quick-and-dirty reset: write 0's to all 245 registers
    for (uint8_t reg = 1; reg < 0xF5; ++reg)
    {
        Adlib_Write(reg, 0);
    }

    terminal_writestring("Done\n");
}

void Adlib_Test(void)
{
    Adlib_Reset();

    // make some goddamned sound!
    Adlib_Write(0x20, 0x01);
    Adlib_Write(0x40, 0x10);
    Adlib_Write(0x60, 0xF0);
    Adlib_Write(0x80, 0x77);
    Adlib_Write(0xA0, 0x98);
    Adlib_Write(0x23, 0x01);
    Adlib_Write(0x43, 0x00);
    Adlib_Write(0x63, 0xF0);
    Adlib_Write(0x83, 0x77);
    Adlib_Write(0xB0, 0x31);

    TimeDelayMS(10000000000);

    // turn the voice off
    Adlib_Write(0xB0, 0x11);
}

void Adlib_Write(uint8_t reg, uint8_t value)
{
    // select the register
    outb(Adlib_BaseAddress, reg);

    // wait 3.3 microseconds
    // TODO TimeDelayMS(1);

    // Write to the register
    outb(Adlib_DataPort, value);

    // We're supposed to wait 23 microseconds before any other sound card operation can be performed
    // (Realistically, waiting doesn't matter because the hardware is probably emulated)
    // TODO TimeDelayMS(1);
}