#pragma once
#include <stdint.h>
#include <stdbool.h>

// Values taken from http://shipbrook.net/jeff/sb.html

#define REG_STATUS          0x00
#define REG_TIMER1_VALUE    0x02    /* Timer1 will be loaded with this value upon reset. */
// This value will be incremented every 80 microseconds until it overflows, which will trigger a TIMER interrupt. */
#define REG_TIMER2_VALUE    0x03    /* Timer2 will be loaded with this value upon reset. */
// This value will be incremented every 320 microseconds until it overflows, which will trigger a TIMER interrupt. */
#define REG_TIMER_CONTROL   0x04

// status definitions
#define STATUS_TIMER_FIRED  0x80
#define STATUS_TIMER1_FIRED 0x40
#define STATUS_TIMER2_FIRED 0x20

// Timer control definitions
#define TIMER_CTRL_IRQ_RESET    0x80    /* Resets flags for timers 1 & 2. If set, all other bits are ignored */
#define TIMER_CTRL_TIMER1_MASK  0x40
#define TIMER_CTRL_TIMER2_MASK  0x20
#define TIMER_CTRL_TIMER2_GO    0x02
#define TIMER_CTRL_TIMER1_GO    0x01

bool Adlib_Init(void);

uint8_t Adlib_Read(uint8_t reg);

void Adlib_Test(void);

void Adlib_Write(uint8_t reg, uint8_t value);