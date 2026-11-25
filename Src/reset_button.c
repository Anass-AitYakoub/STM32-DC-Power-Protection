
#include "reset_button.h"

#include <stdint.h>

#define RCC_AHB1ENR (*(volatile uint32_t *)0x40023830UL)

#define GPIOC_MODER (*(volatile uint32_t *)0x40020800UL)
#define GPIOC_PUPDR (*(volatile uint32_t *)0x4002080CUL)
#define GPIOC_IDR   (*(volatile uint32_t *)0x40020810UL)

#define BUTTON_PIN 13U

void ResetButton_GPIOInit(void)
{
    /* Enable GPIOC peripheral clock. */
    RCC_AHB1ENR |= (1U << 2);
    (void)RCC_AHB1ENR;

    /* Configure PC13 as an input. */
    GPIOC_MODER &= ~(3U << (BUTTON_PIN * 2U));

    /* Enable internal pull-up on PC13. */
    GPIOC_PUPDR &= ~(3U << (BUTTON_PIN * 2U));
    GPIOC_PUPDR |= (1U << (BUTTON_PIN * 2U));
}

bool ResetButton_ReadRaw(void)
{
    /*
     * B1 USER is active-low on this board.
     * LOW = pressed, HIGH = released.
     */
    return (GPIOC_IDR & (1U << BUTTON_PIN)) == 0U;
}

void ResetButton_Init(ResetButton *button)
{
    if (button == 0)
        return;

    button->stable_pressed = false;
    button->last_raw_pressed = false;
    button->initialized = false;
    button->stable_time_ms = 0U;
}

bool ResetButton_Update(
    ResetButton *button,
    bool raw_pressed,
    uint32_t elapsed_ms
)
{
    if (button == 0)
        return false;

    /* Initialize from the first observed input state. */
    if (!button->initialized)
    {
        button->last_raw_pressed = raw_pressed;
        button->stable_pressed = raw_pressed;
        button->stable_time_ms = 0U;
        button->initialized = true;
        return false;
    }

    /* Restart debounce timing when the raw state changes. */
    if (raw_pressed != button->last_raw_pressed)
    {
        button->last_raw_pressed = raw_pressed;
        button->stable_time_ms = 0U;
        return false;
    }

    if (button->stable_time_ms < RESET_DEBOUNCE_MS)
    {
        uint32_t remaining =
            RESET_DEBOUNCE_MS - button->stable_time_ms;

        if (elapsed_ms >= remaining)
            button->stable_time_ms = RESET_DEBOUNCE_MS;
        else
            button->stable_time_ms += elapsed_ms;
    }

    if (button->stable_time_ms >= RESET_DEBOUNCE_MS &&
        button->stable_pressed != raw_pressed)
    {
        button->stable_pressed = raw_pressed;

        /* Generate one reset event on a valid press. */
        return raw_pressed;
    }

    return false;
}
