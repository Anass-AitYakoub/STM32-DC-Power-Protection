
#ifndef SYSTEM_TIMER_H
#define SYSTEM_TIMER_H

#include <stdint.h>

#define SYSTEM_CLOCK_HZ 16000000UL
#define SYSTEM_TICK_MS 10U

/* Initialize SysTick for a 10 ms period. */
void SystemTimer_Init(void);

/* Wait for the next 10 ms SysTick event. */
void SystemTimer_WaitTick(void);

#endif
