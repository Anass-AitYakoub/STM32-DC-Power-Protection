
/**
 ******************************************************************************
 * @file    protection.c
 * @brief   STM32 Smart DC Power Protection System
 *
 * Voltage protection state machine with:
 * - Undervoltage and overvoltage detection
 * - Fault confirmation delay
 * - Startup voltage validation
 * - Latched protection faults
 * - Critical fault detection integration
 * - Manual reset handling
 ******************************************************************************
 */

#include "protection.h"

#include <math.h>
#include <stdint.h>

/* Check whether voltage is within the reset recovery range. */
static bool Voltage_IsRecovered(float voltage)
{
    return voltage >= RESET_MIN_VOLTAGE &&
           voltage <= RESET_MAX_VOLTAGE;
}

/* Detect ordinary voltage faults. */
static ProtectionState DetectFault(float voltage)
{
    if (voltage < UNDERVOLTAGE_THRESHOLD)
    {
        return PROTECTION_UNDERVOLTAGE;
    }

    if (voltage > OVERVOLTAGE_THRESHOLD)
    {
        return PROTECTION_OVERVOLTAGE;
    }

    return PROTECTION_NORMAL;
}

/* Safely accumulate elapsed time without integer overflow. */
static uint32_t AddElapsed(uint32_t current, uint32_t elapsed)
{
    if (elapsed > UINT32_MAX - current)
    {
        return UINT32_MAX;
    }

    return current + elapsed;
}

/* Initialize protection system in a safe state. */
void Protection_Init(ProtectionSystem *system)
{
    if (system == 0)
    {
        return;
    }

    system->state = PROTECTION_STARTUP;
    system->load_enabled = false;

    system->fault_timer_ms = 0U;
    system->startup_timer_ms = 0U;

    system->pending_fault = PROTECTION_NORMAL;
    system->critical_reason = CRITICAL_FAULT_NONE;
}

/*
 * Legacy update function.
 * Retained for compatibility with initial software tests.
 */
void Protection_Update(
    ProtectionSystem *system,
    float voltage,
    bool sensor_valid,
    bool reset_requested
)
{
    if (system == 0)
    {
        return;
    }

    /* Preserve an already latched critical fault. */
    if (system->state == PROTECTION_CRITICAL_FAULT)
    {
        system->load_enabled = false;
        return;
    }

    /* Invalid measurements force the load OFF. */
    if (!sensor_valid || !isfinite(voltage))
    {
        system->state = PROTECTION_SENSOR_FAULT;
        system->load_enabled = false;
        return;
    }

    if (system->state == PROTECTION_STARTUP)
    {
        system->state = DetectFault(voltage);

        system->load_enabled =
            (system->state == PROTECTION_NORMAL);

        return;
    }

    if (system->state != PROTECTION_NORMAL)
    {
        system->load_enabled = false;

        if (reset_requested && Voltage_IsRecovered(voltage))
        {
            system->state = PROTECTION_NORMAL;
            system->load_enabled = true;
        }

        return;
    }

    system->state = DetectFault(voltage);

    system->load_enabled =
        (system->state == PROTECTION_NORMAL);
}

/*
 * Time-based protection update.
 *
 * Ordinary voltage faults require confirmation.
 * Critical faults are handled separately by
 * Protection_TripCritical().
 */
void Protection_UpdateTimed(
    ProtectionSystem *system,
    float voltage,
    bool sensor_valid,
    bool reset_requested,
    uint32_t elapsed_ms
)
{
    if (system == 0)
    {
        return;
    }

    /*
     * Preserve any latched critical fault.
     * A valid manual reset is required to clear it.
     */
    if (system->state == PROTECTION_CRITICAL_FAULT)
    {
        system->load_enabled = false;

        if (sensor_valid &&
            isfinite(voltage) &&
            reset_requested &&
            Voltage_IsRecovered(voltage))
        {
            system->state = PROTECTION_STARTUP;
            system->load_enabled = false;

            system->startup_timer_ms = 0U;
            system->fault_timer_ms = 0U;
            system->pending_fault = PROTECTION_NORMAL;
            system->critical_reason = CRITICAL_FAULT_NONE;
        }

        return;
    }

    /* Invalid measurements must shut down the load. */
    if (!sensor_valid || !isfinite(voltage))
    {
        system->state = PROTECTION_SENSOR_FAULT;
        system->load_enabled = false;

        system->fault_timer_ms = 0U;
        system->startup_timer_ms = 0U;

        return;
    }

    /* Validate stable voltage before enabling the load. */
    if (system->state == PROTECTION_STARTUP)
    {
        system->load_enabled = false;

        if (Voltage_IsRecovered(voltage))
        {
            system->startup_timer_ms = AddElapsed(
                system->startup_timer_ms,
                elapsed_ms
            );

            if (system->startup_timer_ms >=
                STARTUP_CONFIRMATION_MS)
            {
                system->state = PROTECTION_NORMAL;
                system->load_enabled = true;
            }
        }
        else
        {
            system->startup_timer_ms = 0U;
        }

        return;
    }

    /* Ordinary faults remain latched until manual reset. */
    if (system->state != PROTECTION_NORMAL)
    {
        system->load_enabled = false;

        if (reset_requested && Voltage_IsRecovered(voltage))
        {
            /* Return to startup validation, not directly ON. */
            system->state = PROTECTION_STARTUP;
            system->load_enabled = false;

            system->startup_timer_ms = 0U;
            system->fault_timer_ms = 0U;
            system->pending_fault = PROTECTION_NORMAL;
            system->critical_reason = CRITICAL_FAULT_NONE;
        }

        return;
    }

    /* Detect ordinary voltage faults. */
    ProtectionState detected = DetectFault(voltage);

    if (detected == PROTECTION_NORMAL)
    {
        system->fault_timer_ms = 0U;
        system->pending_fault = PROTECTION_NORMAL;
        system->load_enabled = true;

        return;
    }

    /* Restart the timer if the fault type changes. */
    if (detected != system->pending_fault)
    {
        system->pending_fault = detected;
        system->fault_timer_ms = 0U;
    }

    /* Accumulate the fault duration. */
    system->fault_timer_ms = AddElapsed(
        system->fault_timer_ms,
        elapsed_ms
    );

    /* Confirm the ordinary fault after the delay. */
    if (system->fault_timer_ms >= FAULT_CONFIRMATION_MS)
    {
        system->state = detected;
        system->load_enabled = false;
    }
}

/*
 * Immediately latch a critical fault.
 *
 * This function overrides ordinary protection logic.
 * The first critical fault reason is preserved.
 */
void Protection_TripCritical(
    ProtectionSystem *system,
    CriticalFault fault
)
{
    if (system == 0 || fault == CRITICAL_FAULT_NONE)
    {
        return;
    }

    /* Preserve the first detected critical fault. */
    if (system->state != PROTECTION_CRITICAL_FAULT)
    {
        system->critical_reason = fault;
    }

    /* Force the load command OFF immediately. */
    system->state = PROTECTION_CRITICAL_FAULT;
    system->load_enabled = false;

    /* Cancel pending confirmations. */
    system->fault_timer_ms = 0U;
    system->startup_timer_ms = 0U;
    system->pending_fault = PROTECTION_NORMAL;
}
