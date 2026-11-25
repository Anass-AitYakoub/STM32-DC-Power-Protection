
/**
 ******************************************************************************
 * @file    main.c
 * @brief   STM32 Smart DC Power Protection System
 *
 * Phase 15: ADC and Critical Fault Integration
 *
 * - Real ADC measurements on PA0
 * - EMA voltage filtering
 * - Critical fault detection
 * - Latched protection state machine
 * - Debounced manual reset button
 *
 * Physical MOSFET control remains disabled.
 ******************************************************************************
 */

#include <stdbool.h>
#include <stdint.h>

#include "adc_reader.h"
#include "voltage_monitor.h"
#include "critical_fault.h"
#include "protection.h"
#include "board_led.h"
#include "reset_button.h"
#include "system_timer.h"

/* Global variables for debugger inspection. */
volatile uint16_t debug_adc_raw = 0U;

volatile float debug_input_voltage = 0.0f;
volatile float debug_filtered_voltage = 0.0f;

volatile bool debug_adc_read_ok = false;
volatile bool debug_sensor_valid = false;
volatile bool debug_reset_event = false;

volatile ProtectionState debug_state = PROTECTION_STARTUP;
volatile CriticalFault debug_critical_fault = CRITICAL_FAULT_NONE;
volatile CriticalFault debug_latched_reason = CRITICAL_FAULT_NONE;

volatile bool debug_load_permission = false;

volatile uint32_t debug_sample_count = 0U;
volatile uint32_t debug_adc_errors = 0U;

int main(void)
{
    ProtectionSystem protection;
    VoltageFilter input_filter;
    ResetButton reset_button;

    /* Initialize software modules. */
    Protection_Init(&protection);
    VoltageMonitor_FilterInit(&input_filter);
    ResetButton_Init(&reset_button);

    /* Initialize STM32 peripherals. */
    BoardLED_Init();
    BoardLED_Set(false);

    ResetButton_GPIOInit();
    ADCReader_Init();
    SystemTimer_Init();

    while (1)
    {
        /* Wait for the next 10 ms timer event. */
        SystemTimer_WaitTick();

        /* Read and debounce the user reset button. */
        bool raw_pressed = ResetButton_ReadRaw();

        bool reset_event = ResetButton_Update(
            &reset_button,
            raw_pressed,
            SYSTEM_TICK_MS
        );

        /* Acquire a real ADC sample. */
        uint16_t adc_raw = 0U;
        bool adc_ok = ADCReader_Read(&adc_raw);

        /* Validate the ADC sample. */
        bool sensor_valid =
            adc_ok && VoltageMonitor_IsValid(adc_raw);

        float raw_voltage = 0.0f;
        float filtered_voltage = 0.0f;

        if (sensor_valid)
        {
            raw_voltage =
                VoltageMonitor_ConvertADC(adc_raw);
        }

        /*
         * Check critical conditions using raw voltage,
         * before applying the EMA filter.
         */
        CriticalFault critical_fault = CriticalFault_Check(
            adc_ok,
            sensor_valid,
            raw_voltage
        );

        if (CriticalFault_IsActive(critical_fault))
        {
            /* Latch the critical fault immediately. */
            Protection_TripCritical(
                &protection,
                critical_fault
            );

            /* Discard previous filtered measurements. */
            VoltageMonitor_FilterInit(&input_filter);
        }
        else
        {
            /* Filter only valid measurements. */
            filtered_voltage =
                VoltageMonitor_FilterUpdate(
                    &input_filter,
                    raw_voltage
                );

            /*
             * Update ordinary protection logic,
             * startup validation, and manual reset.
             */
            Protection_UpdateTimed(
                &protection,
                filtered_voltage,
                true,
                reset_event,
                SYSTEM_TICK_MS
            );
        }

        /* Keep physical outputs disabled. */
        BoardLED_Set(false);

        /* Update debugger variables. */
        debug_adc_raw = adc_raw;
        debug_adc_read_ok = adc_ok;
        debug_sensor_valid = sensor_valid;

        debug_input_voltage = raw_voltage;
        debug_filtered_voltage = filtered_voltage;

        debug_reset_event = reset_event;
        debug_state = protection.state;

        debug_critical_fault = critical_fault;
        debug_latched_reason = protection.critical_reason;

        debug_load_permission = protection.load_enabled;

        debug_sample_count++;

        if (!adc_ok)
        {
            debug_adc_errors++;
        }
    }
}
