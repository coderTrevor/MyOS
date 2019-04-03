#pragma once
#include <stdint.h>
#include <stdbool.h>

extern uint64_t ticksSinceReset;
extern uint64_t nanosecondsPerTick;
extern uint64_t microsecondsPerTick;
extern uint64_t millisecondsPerTick;
extern uint64_t ticksPerHour;
extern uint64_t ticksPerMinute;
extern uint64_t ticksPerSecond;

void TimeDelayMS(uint64_t milliseconds);

void TimeGetTimeSinceReset(int *pHours, int *pMinutes, int *pSeconds);

// Format time in the form hh:mm:ss
void TimeFormatTimeString(char *timeString);

extern bool showClock;