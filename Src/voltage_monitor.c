
#include "voltage_monitor.h"

float VoltageMonitor_ConvertADC(uint16_t adc_raw)
{
    float adc_voltage =
        ((float)adc_raw / ADC_MAX_VALUE) * ADC_REFERENCE_V;

    float measured_voltage =
        adc_voltage *
        ((VOLTAGE_R1_OHMS + VOLTAGE_R2_OHMS) /
         VOLTAGE_R2_OHMS);

    return measured_voltage;
}

bool VoltageMonitor_IsValid(uint16_t adc_raw)
{
    if (adc_raw > ADC_MAX_VALUE)
    {
        return false;
    }

    float voltage = VoltageMonitor_ConvertADC(adc_raw);

    return (voltage >= 0.0f &&
            voltage <= MAX_MEASURED_VOLTAGE);
}

void VoltageMonitor_FilterInit(VoltageFilter *filter)
{
    if (filter == 0)
        return;

    filter->filtered_voltage = 0.0f;
    filter->initialized = false;
}

float VoltageMonitor_FilterUpdate(
    VoltageFilter *filter,
    float new_voltage
)
{
    if (filter == 0)
        return new_voltage;

    /* Initialize the filter with the first measurement. */
    if (!filter->initialized)
    {
        filter->filtered_voltage = new_voltage;
        filter->initialized = true;
        return new_voltage;
    }

    /* Apply exponential moving average filtering. */
    filter->filtered_voltage =
        VOLTAGE_FILTER_ALPHA * new_voltage +
        (1.0f - VOLTAGE_FILTER_ALPHA) *
        filter->filtered_voltage;

    return filter->filtered_voltage;
}
