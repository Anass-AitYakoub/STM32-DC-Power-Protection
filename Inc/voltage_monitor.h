
#ifndef VOLTAGE_MONITOR_H
#define VOLTAGE_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

#define ADC_MAX_VALUE       4095U
#define ADC_REFERENCE_V     3.3f

#define VOLTAGE_R1_OHMS     100000.0f
#define VOLTAGE_R2_OHMS     10000.0f

#define MAX_MEASURED_VOLTAGE 24.0f

float VoltageMonitor_ConvertADC(uint16_t adc_raw);

bool VoltageMonitor_IsValid(uint16_t adc_raw);


/* Exponential Moving Average filter. */
#define VOLTAGE_FILTER_ALPHA 0.2f

typedef struct
{
    float filtered_voltage;
    bool initialized;
} VoltageFilter;

void VoltageMonitor_FilterInit(VoltageFilter *filter);

float VoltageMonitor_FilterUpdate(
    VoltageFilter *filter,
    float new_voltage
);

#endif
