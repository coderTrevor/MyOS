// Functions for handling the programmable interval timer, AKA 8253/8254 chip
#include "PIT.h"
#include "System_Clock.h"
#include "../Interrupts/Interrupts.h"
#include "../Interrupts/PIC.h"

// Interrupt handler for the PIT
void _declspec(naked) timer_interrupt_handler(void)
{
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