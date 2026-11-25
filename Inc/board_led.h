#ifndef BOARD_LED_H
#define BOARD_LED_H

#include <stdbool.h>

/* Initialize the NUCLEO-F401RE onboard LED (PA5). */
void BoardLED_Init(void);

/* Control the onboard LED. */
void BoardLED_Set(bool enabled);

#endif
