
#include "load_control.h"

void LoadControl_Init(LoadControl *control)
{
    if (control == 0)
        return;

    /* Default to a safe disconnected state. */
    control->load_enabled = false;
    control->gpio_high = false;
}

void LoadControl_Update(
    LoadControl *control,
    bool protection_allows_load
)
{
    if (control == 0)
        return;

    /*
     * Logical GPIO command:
     * HIGH = enable MOSFET driver
     * LOW  = disable MOSFET driver
     */
    control->load_enabled = protection_allows_load;
    control->gpio_high = protection_allows_load;
}
