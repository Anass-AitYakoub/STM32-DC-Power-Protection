
#ifndef RESET_BUTTON_H
#define RESET_BUTTON_H

#include <stdbool.h>
#include <stdint.h>

#define RESET_DEBOUNCE_MS 30U

typedef struct
{
    bool stable_pressed;
    bool last_raw_pressed;
    bool initialized;
    uint32_t stable_time_ms;
} ResetButton;

/* Initialize the PC13 user button input. */
void ResetButton_GPIOInit(void);

/* Read the physical button state. */
bool ResetButton_ReadRaw(void);

/* Initialize the debounce state. */
void ResetButton_Init(ResetButton *button);

/*
 * Return true once for each valid button press.
 * elapsed_ms must represent real elapsed time.
 */
bool ResetButton_Update(
    ResetButton *button,
    bool raw_pressed,
    uint32_t elapsed_ms
);

#endif
