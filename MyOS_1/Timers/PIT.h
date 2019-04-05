#pragma once

#include "../Interrupts/Interrupts.h"
#include <stdint.h>

// I/O Ports
#define PIT_PORT_CHANNEL_0           0x40
#define PIT_PORT_CHANNEL_1           0x41   /* Not really used; may not even be implemented */ 
#define PIT_PORT_CHANNEL_2           0x42   /* Used by pc speaker */
#define PIT_PORT_COMMAND_REG         0x43

// Mode/command register bits
// bits 6 and 7, channel select
#define PIT_MODE_CHANNEL_0      0
#define PIT_MODE_CHANNEL_1      0x40    /* Not really used; may not even be implemented */
#define PIT_MODE_CHANNEL_2      0x80    /* Used by pc speaker */
#define PIT_MODE_READ_BACK_CMD  0xC0

// bits 4 and 5, access mode
#define PIT_MODE_LATCH_COUNT        0
#define PIT_MODE_ACCESS_LOW_BYTE    0x10
#define PIT_MODE_ACCESS_HIGH_BYTE   0x20
#define PIT_MODE_ACCESS_BOTH_BYTES  0x30

// bits 1-3, operating mode
#define PIT_MODE_INTERRUPT_ON_TERM_COUNT        0
#define PIT_MODE_HARDWARE_TRIGGERABLE_ONE_SHOT  2
#define PIT_MODE_RATE_GENERATOR                 4
#define PIT_MODE_SQUARE_WAVE_GENERATOR          6
#define PIT_MODE_SOFTWARE_TRIGGERED_STROBE      8
#define PIT_MODE_HARDWARE_TRIGGERED_STROBE      10

// bit 0, counting mode
#define PIT_MODE_BINARY         0
#define PIT_MODE_BCD            1   /* Not really useful */

#define TIMER_INTERRUPT (HARDWARE_INTERRUPTS_BASE + 0) /* remapped interrupt value for PIT */

#define PIT_INPUT_CLOCK             1193182   /* This is the approximate clock value in hz that the PIT ends up dividing */

// Using this value divided by 12 *may* lead to more accurate tick-count calculations (I'm not sure):
#define PIT_INPUT_CLOCK_TIMES_12    14318180  /* This is the number that is divided by 12 to arrive at the PIT input.*/

// Functions:
void PIT_Set_Interval(uint32_t hz);

void timer_interrupt_handler(void);
