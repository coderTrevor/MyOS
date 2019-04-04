#include "System_Clock.h"
#include <stdbool.h>
#include "../misc.h"

// system clock defaults to one "tick" (IRQ0 firing) at a rate of 18.222052535125229077580940745266 hz
uint64_t ticksSinceReset = 0;

// These values are approximate. They will need to be updated if we change the PIT's divider
// If we need more accuracy it may be desirable to convert some of these to fixed point or floating point,
// but my OS doesn't need accurate timing as of right now.
uint64_t nanosecondsPerTick = 54878718;
uint64_t microsecondsPerTick = 54879;
uint64_t millisecondsPerTick = 55;
uint64_t ticksPerHour = 65599;
uint64_t ticksPerMinute = 1093;
uint64_t ticksPerSecond = 18;

bool showClock = true;

void TimeDelayMS(uint64_t milliseconds)
{
    volatile uint64_t oldTicks = ticksSinceReset;

    // figure out how many ticks we need to wait
    uint64_t waitTicks = milliseconds / millisecondsPerTick;

    // We don't know how soon ticksSinceReset will be updated, and we want to wait at least until the given time indicated
    // (TODO: improve accuracy somehow?)
    // Wait until ticks changes
    while (oldTicks == ticksSinceReset)
        ;

    // update oldTicks
    oldTicks = ticksSinceReset;

    // wait
    for (volatile uint64_t ticks = 0; ticks < waitTicks; ++ticks)
        ;
}

// Format time in the form hh:mm:ss (time since reset)
// timeString must be at least 9 bytes long
void TimeFormatTimeString(char *timeString)
{
    int hours, minutes, seconds;
    TimeGetTimeSinceReset(&hours, &minutes, &seconds);

    int digit1 = hours / 10;
    int digit2 = hours % 10;

    timeString[0] = intToChar(digit1);
    timeString[1] = intToChar(digit2);
    timeString[2] = ':';

    digit1 = minutes / 10;
    digit2 = minutes % 10;
    timeString[3] = intToChar(digit1);
    timeString[4] = intToChar(digit2);
    timeString[5] = ':';

    digit1 = seconds / 10;
    digit2 = seconds % 10;
    timeString[6] = intToChar(digit1);
    timeString[7] = intToChar(digit2);
    timeString[8] = '\0';
}

void TimeGetTimeSinceReset(int *pHours, int *pMinutes, int *pSeconds)
{
    // It would be innacurate to calculate seconds based solely on ticksSinceReset and ticksPerSecond.
    // it's more accurate to calculate hours, then get minutes based on the remainder of the hours calc, the get seconds based
    // on the remainder of the minutes calc
    // Note: While doing it this way should be more "accurate" with large tick counts it sometimes has weird results, 
    // like staying on 00:01:00 for noticably longer than one second
    uint64_t ticksRemainder = ticksSinceReset;

    *pHours = (int)(ticksRemainder / ticksPerHour);
    ticksRemainder = ticksRemainder % ticksPerHour;

    *pMinutes = (int)(ticksRemainder / ticksPerMinute);
    ticksRemainder = ticksRemainder % ticksPerMinute;

    *pSeconds = (int)(ticksRemainder / ticksPerSecond);
    
    // Try to mitigate the inevitable inaccuracies
    if (*pSeconds >= 60)
    {
        *pSeconds -= 60;
        ++*pMinutes;
    }

    if (*pMinutes >= 60)
    {
        *pMinutes -= 60;
        ++*pHours;
    }
}

/*uint64_t GetSecondsSinceReset()
{
return ticksSinceReset / ticksPerSecond;
}*/