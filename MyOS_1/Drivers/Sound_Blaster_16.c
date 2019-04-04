#include <stdbool.h>
#include "Sound_Blaster_16.h"
#include "../Terminal.h"
#include "../System_Specific.h"
#include "../Timers/System_Clock.h"

bool sb16Present = false;
uint16_t sb16BaseAddress = 0x220;
uint8_t sb16VersionMajor;
uint8_t sb16VersionMinor;

bool SB16_Reset(void)
{
    sb16Present = false;

    // send reset command
    outb(sb16BaseAddress + DSP_RESET, 1);
    
    // wait (much more than) 3.3 microseconds 
    TimeDelayMS(1);

    outb(sb16BaseAddress + DSP_RESET, 0);

    // if present, the DSP should respond within 100 microseconds    
    // TODO: determine how many ticks need to go by for 100 microseconds and don't wait any longer than that
    TimeDelayMS(1);
    if (!(inb(sb16BaseAddress + DSP_STATUS) & READ_WRITE_READY_BIT))
        return false;

    uint8_t readValue = inb(sb16BaseAddress + DSP_READ);
    if (readValue == DSP_READY)
        sb16Present = true;

    return sb16Present;
}

void SB16_Init(void)
{
    terminal_writestring("SB16 driver is looking for a SoundBlaster compatible card...\n");

    sb16BaseAddress = SB16_BASE0;
    
    // Try to send the DSP reset command
    if (!SB16_Reset())
    {
        // Failed to reset the DSP, try the next address
        sb16BaseAddress = SB16_BASE1;
        if (!SB16_Reset())
        {
            // Failed to reset the DSP again, try the final address
            sb16BaseAddress = SB16_BASE2;
            SB16_Reset();
        }
    }

    if (!sb16Present)
    {
        terminal_writestring("No SoundBlaster 16 driver was found.\n");
        return;
    }

    // Get the DSP version
    SB16_Write(DSP_CMD_VERSION);
    sb16VersionMajor = SB16_Read();
    sb16VersionMinor = SB16_Read();

    terminal_writestring("SoundBlaster 16 found at ");
    terminal_print_ushort_hex(sb16BaseAddress);
    terminal_writestring(", DSP Version ");
    terminal_print_int(sb16VersionMajor);
    terminal_putchar('.');
    terminal_print_int(sb16VersionMinor);
    terminal_writestring(", initialized.\n");    
}

uint8_t SB16_Read()
{
    // TODO: timeout after a while
    // wait until the DSP is ready to be read (wait until ready bit is set)
    while (!inb(sb16BaseAddress + DSP_STATUS) & READ_WRITE_READY_BIT)
        ;

    return inb(sb16BaseAddress + DSP_READ);
}

void SB16_Write(uint8_t data)
{
    // TODO: timeout after a while
    // wait until the DSP is ready to be written to (wait until ready bit is cleared)
    while (inb(sb16BaseAddress + DSP_WRITE) & READ_WRITE_READY_BIT)
        ;

    outb(sb16BaseAddress + DSP_WRITE, data);
}
