
#ifndef HARDWARE_IO_H
#define HARDWARE_IO_H

#include <stdbool.h>

#include "protection.h"

typedef struct
{
    bool mosfet_output;
    bool normal_led;
    bool fault_led;
    bool reset_button;
} HardwareIO;

/* Initialize all outputs in a safe state. */
void HardwareIO_Init(HardwareIO *io);

/* Update the logical hardware outputs. */
void HardwareIO_Update(
    HardwareIO *io,
    const ProtectionSystem *protection,
    bool load_command
);

/* Read the logical reset button state. */
bool HardwareIO_ReadReset(const HardwareIO *io);

#endif
