
#ifndef CRITICAL_FAULT_H
#define CRITICAL_FAULT_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Provisional thresholds for software development.
 * Must be validated against real hardware requirements.
 */
#define CRITICAL_UNDERVOLTAGE_V 9.0f
#define CRITICAL_OVERVOLTAGE_V  16.0f

typedef enum
{
    CRITICAL_FAULT_NONE = 0,
    CRITICAL_FAULT_ADC_FAILURE,
    CRITICAL_FAULT_INVALID_READING,
    CRITICAL_FAULT_UNDERVOLTAGE,
    CRITICAL_FAULT_OVERVOLTAGE
} CriticalFault;

/*
 * Check raw input voltage before EMA filtering.
 * Returns the detected critical fault.
 */
CriticalFault CriticalFault_Check(
    bool adc_read_ok,
    bool sensor_valid,
    float raw_voltage
);

bool CriticalFault_IsActive(CriticalFault fault);

#endif
