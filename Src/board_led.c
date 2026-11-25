
#include "board_led.h"
#include <stdint.h>

/*
 * STM32F401RE register definitions.
 * Onboard LED LD2 is connected to PA5.
 */

#define RCC_AHB1ENR  (*(volatile uint32_t *)0x40023830UL)

#define GPIOA_MODER  (*(volatile uint32_t *)0x40020000UL)
#define GPIOA_OTYPER (*(volatile uint32_t *)0x40020004UL)
#define GPIOA_PUPDR  (*(volatile uint32_t *)0x4002000CUL)
#define GPIOA_BSRR   (*(volatile uint32_t *)0x40020018UL)

#define LED_PIN 5U

void BoardLED_Init(void)
{
    /* Enable GPIOA peripheral clock. */
    RCC_AHB1ENR |= (1U << 0);

    /* Ensure the peripheral clock is active. */
    (void)RCC_AHB1ENR;

    /* Prepare the LED output in the OFF state. */
    GPIOA_BSRR = (1U << (LED_PIN + 16U));

    /* Configure PA5 as general-purpose output. */
    GPIOA_MODER &= ~(3U << (LED_PIN * 2U));
    GPIOA_MODER |=  (1U << (LED_PIN * 2U));

    /* Configure push-pull output. */
    GPIOA_OTYPER &= ~(1U << LED_PIN);

    /* Disable internal pull-up and pull-down. */
    GPIOA_PUPDR &= ~(3U << (LED_PIN * 2U));
}

void BoardLED_Set(bool enabled)
{
    if (enabled)
    {
        /* Set PA5 HIGH: LED ON. */
        GPIOA_BSRR = (1U << LED_PIN);
    }
    else
    {
        /* Set PA5 LOW: LED OFF. */
        GPIOA_BSRR = (1U << (LED_PIN + 16U));
    }
}
