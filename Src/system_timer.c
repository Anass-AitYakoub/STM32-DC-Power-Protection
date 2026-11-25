
#include "system_timer.h"

#include <stdint.h>

/* ARM Cortex-M4 SysTick registers. */
#define SYST_CSR (*(volatile uint32_t *)0xE000E010UL)
#define SYST_RVR (*(volatile uint32_t *)0xE000E014UL)
#define SYST_CVR (*(volatile uint32_t *)0xE000E018UL)

#define SYSTICK_ENABLE    (1U << 0)
#define SYSTICK_CLKSOURCE (1U << 2)
#define SYSTICK_COUNTFLAG (1U << 16)

#define SYSTICK_RELOAD \
    ((SYSTEM_CLOCK_HZ / 1000UL) * SYSTEM_TICK_MS - 1UL)

void SystemTimer_Init(void)
{
    /* Stop SysTick before configuration. */
    SYST_CSR = 0U;

    /* Set the 10 ms reload value. */
    SYST_RVR = SYSTICK_RELOAD;

    /* Clear the current counter value. */
    SYST_CVR = 0U;

    /*
     * Enable SysTick using the processor clock.
     * Interrupts remain disabled.
     */
    SYST_CSR = SYSTICK_ENABLE | SYSTICK_CLKSOURCE;
}

void SystemTimer_WaitTick(void)
{
    /* Wait for the next SysTick counter rollover. */
    while ((SYST_CSR & SYSTICK_COUNTFLAG) == 0U)
    {
        /* Wait for the hardware timer. */
    }
}
