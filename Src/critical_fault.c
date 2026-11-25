
#include "critical_fault.h"

#include <math.h>

CriticalFault CriticalFault_Check(
    bool adc_read_ok,
    bool sensor_valid,
    float raw_voltage
)
{
    /* ADC communication or conversion failure. */
    if (!adc_read_ok)
    {
        return CRITICAL_FAULT_ADC_FAILURE;
    }

    /* Invalid, infinite, or undefined voltage reading. */
    if (!sensor_valid || !isfinite(raw_voltage))
    {
        return CRITICAL_FAULT_INVALID_READING;
    }

    /* Check critical undervoltage. */
    if (raw_voltage < CRITICAL_UNDERVOLTAGE_V)
    {
        return CRITICAL_FAULT_UNDERVOLTAGE;
    }

    /* Check critical overvoltage. */
    if (raw_voltage > CRITICAL_OVERVOLTAGE_V)
    {
        return CRITICAL_FAULT_OVERVOLTAGE;
    }

    return CRITICAL_FAULT_NONE;
}

bool CriticalFault_IsActive(CriticalFault fault)
{
    return fault != CRITICAL_FAULT_NONE;
}
