#ifndef LOAD_CONTROL_H
#define LOAD_CONTROL_H

#include <stdbool.h>

typedef struct
{
    bool load_enabled;
    bool gpio_high;
} LoadControl;

void LoadControl_Init(LoadControl *control);

void LoadControl_Update(
    LoadControl *control,
    bool protection_allows_load
);

#endif
