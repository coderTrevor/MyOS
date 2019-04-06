// Functions for handling the programmable interval timer, AKA 8253/8254 chip
#include "PIT.h"
#include "System_Clock.h"
#include "../Interrupts/Interrupts.h"
#include "../Interrupts/PIC.h"
#include "../System_Specific.h"
#include "../File Formats/VOC.h"
#include "../Drivers/Sound_Blaster_16.h"

// TEMPTEMP
bool playSound = false;
extern Sound_Struct sounds[2];
extern unsigned int soundIndex;

// TODO: Round numbers up as needed to improve accuracy
void PIT_Set_Interval(uint32_t hz)
{
    uint16_t divisor = 0;

    // Calculate divisor.
    // Make sure the requested speed isn't too slow
    if (hz < (PIT_INPUT_CLOCK / 65536))
    {
        hz = PIT_INPUT_CLOCK / 65536;
        divisor = 0;
    }
    else
    {
        // make sure the requested value isn't too fast
        if (hz > PIT_INPUT_CLOCK)
        {
            hz = PIT_INPUT_CLOCK;
            divisor = 1;
        }
        else
        {
            divisor = (uint16_t)(PIT_INPUT_CLOCK / hz);
        }
    }

    // recalculate tick counts based on requested input
    // TODO: Accuracy can be considerably improved
    nanosecondsPerTick  = 1000000000 / hz;
    microsecondsPerTick = 1000000 / hz;
    millisecondsPerTick = 1000 / hz;

    ticksPerHour = hz * 60 * 60;
    ticksPerMinute = hz * 60;
    ticksPerSecond = hz;

    // Now we need to program the new divisor into the PIT's channel 0
    _disable();
    outb(PIT_PORT_COMMAND_REG, PIT_MODE_BINARY | PIT_MODE_SQUARE_WAVE_GENERATOR | PIT_MODE_ACCESS_BOTH_BYTES | PIT_MODE_CHANNEL_0);
    outb(PIT_PORT_CHANNEL_0, (uint8_t)(divisor & 0xFF));   // Program the low byte
    outb(PIT_PORT_CHANNEL_0, (uint8_t)(divisor >> 8));     // Program the high byte
    _enable();

    // we count this as a reset
    ticksSinceReset = 0;
}

// Interrupt handler for the PIT
void _declspec(naked) timer_interrupt_handler(void)
{
    _disable();
    _asm pushad;

    //++interrupts_fired;
    ++ticksSinceReset;

    PIC_sendEOI(TIMER_INTERRUPT);

    _asm
    {
        popad
        iretd
    }
}