
#include "hardware_io.h"

void HardwareIO_Init(HardwareIO *io)
{
    if (io == 0)
        return;

    /* Safe default state. */
    io->mosfet_output = false;
    io->normal_led = false;
    io->fault_led = false;
    io->reset_button = false;
}

void HardwareIO_Update(
    HardwareIO *io,
    const ProtectionSystem *protection,
    bool load_command
)
{
    if (io == 0)
        return;

    /* Fail-safe behavior if protection information is missing. */
    if (protection == 0)
    {
        io->mosfet_output = false;
        io->normal_led = false;
        io->fault_led = true;
        return;
    }

    /*
     * Only allow load operation when the protection
     * state is NORMAL and both enable signals agree.
     */
    bool allow_load =
        (protection->state == PROTECTION_NORMAL) &&
        protection->load_enabled &&
        load_command;

    io->mosfet_output = allow_load;

    /* Status indication. */
    io->normal_led = allow_load;

    io->fault_led =
        (protection->state == PROTECTION_UNDERVOLTAGE) ||
        (protection->state == PROTECTION_OVERVOLTAGE) ||
        (protection->state == PROTECTION_SENSOR_FAULT);
}

bool HardwareIO_ReadReset(const HardwareIO *io)
{
    if (io == 0)
        return false;

    return io->reset_button;
}
