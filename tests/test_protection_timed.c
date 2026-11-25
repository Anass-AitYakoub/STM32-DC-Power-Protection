
/**
 ******************************************************************************
 * @file    test_protection_timed.c
 * @brief   Automated tests for the DC Power Protection System
 *
 * Tests the actual Embedded C protection and critical fault modules.
 * Designed for execution on a host PC or CI runner.
 ******************************************************************************
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "protection.h"
#include "critical_fault.h"

#define SAMPLE_PERIOD_MS 10U

/* Simulate periodic protection updates. */
static void RunSamples(
    ProtectionSystem *system,
    float voltage,
    unsigned int samples
)
{
    for (unsigned int i = 0U; i < samples; i++)
    {
        Protection_UpdateTimed(
            system,
            voltage,
            true,
            false,
            SAMPLE_PERIOD_MS
        );
    }
}

/* Test 1: Safe initialization. */
static void Test_Startup(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    assert(system.state == PROTECTION_STARTUP);
    assert(system.load_enabled == false);
    assert(system.critical_reason == CRITICAL_FAULT_NONE);
}

/* Test 2: Normal operation after 100 ms. */
static void Test_NormalVoltage(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    RunSamples(&system, 12.0f, 10U);

    assert(system.state == PROTECTION_NORMAL);
    assert(system.load_enabled == true);
}

/* Test 3: Ordinary undervoltage after 100 ms. */
static void Test_Undervoltage(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    RunSamples(&system, 12.0f, 10U);
    RunSamples(&system, 10.0f, 10U);

    assert(system.state == PROTECTION_UNDERVOLTAGE);
    assert(system.load_enabled == false);
}

/* Test 4: Ordinary overvoltage after 100 ms. */
static void Test_Overvoltage(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    RunSamples(&system, 12.0f, 10U);
    RunSamples(&system, 15.0f, 10U);

    assert(system.state == PROTECTION_OVERVOLTAGE);
    assert(system.load_enabled == false);
}

/* Test 5: Reject a short voltage disturbance. */
static void Test_TransientFault(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    RunSamples(&system, 12.0f, 10U);
    RunSamples(&system, 10.0f, 5U);
    RunSamples(&system, 12.0f, 1U);

    assert(system.state == PROTECTION_NORMAL);
    assert(system.load_enabled == true);
    assert(system.fault_timer_ms == 0U);
}

/* Test 6: Critical undervoltage. */
static void Test_CriticalUndervoltage(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    RunSamples(&system, 12.0f, 10U);

    CriticalFault fault =
        CriticalFault_Check(true, true, 8.0f);

    assert(fault == CRITICAL_FAULT_UNDERVOLTAGE);

    Protection_TripCritical(&system, fault);

    assert(system.state == PROTECTION_CRITICAL_FAULT);
    assert(system.load_enabled == false);
    assert(system.critical_reason ==
           CRITICAL_FAULT_UNDERVOLTAGE);
}

/* Test 7: Critical overvoltage. */
static void Test_CriticalOvervoltage(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    RunSamples(&system, 12.0f, 10U);

    CriticalFault fault =
        CriticalFault_Check(true, true, 17.0f);

    assert(fault == CRITICAL_FAULT_OVERVOLTAGE);

    Protection_TripCritical(&system, fault);

    assert(system.state == PROTECTION_CRITICAL_FAULT);
    assert(system.load_enabled == false);
}

/* Test 8: ADC conversion failure. */
static void Test_ADCFailure(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    RunSamples(&system, 12.0f, 10U);

    CriticalFault fault =
        CriticalFault_Check(false, false, 0.0f);

    assert(fault == CRITICAL_FAULT_ADC_FAILURE);

    Protection_TripCritical(&system, fault);

    assert(system.state == PROTECTION_CRITICAL_FAULT);
    assert(system.load_enabled == false);
}

/* Test 9: Critical faults must remain latched. */
static void Test_FaultLatching(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    RunSamples(&system, 12.0f, 10U);

    Protection_TripCritical(
        &system,
        CRITICAL_FAULT_OVERVOLTAGE
    );

    RunSamples(&system, 12.0f, 20U);

    assert(system.state == PROTECTION_CRITICAL_FAULT);
    assert(system.load_enabled == false);
    assert(system.critical_reason ==
           CRITICAL_FAULT_OVERVOLTAGE);
}

/* Test 10: Manual reset and startup validation. */
static void Test_ManualReset(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    RunSamples(&system, 12.0f, 10U);

    Protection_TripCritical(
        &system,
        CRITICAL_FAULT_UNDERVOLTAGE
    );

    /* Reset with recovered voltage. */
    Protection_UpdateTimed(
        &system,
        12.0f,
        true,
        true,
        SAMPLE_PERIOD_MS
    );

    assert(system.state == PROTECTION_STARTUP);
    assert(system.load_enabled == false);
    assert(system.critical_reason == CRITICAL_FAULT_NONE);

    /* Complete the required startup validation. */
    RunSamples(&system, 12.0f, 10U);

    assert(system.state == PROTECTION_NORMAL);
    assert(system.load_enabled == true);
}

/* Run the complete test suite. */
int main(void)
{
    Test_Startup();
    printf("[PASS] Test 1: Safe startup\n");

    Test_NormalVoltage();
    printf("[PASS] Test 2: Normal voltage\n");

    Test_Undervoltage();
    printf("[PASS] Test 3: Undervoltage\n");

    Test_Overvoltage();
    printf("[PASS] Test 4: Overvoltage\n");

    Test_TransientFault();
    printf("[PASS] Test 5: Transient fault\n");

    Test_CriticalUndervoltage();
    printf("[PASS] Test 6: Critical undervoltage\n");

    Test_CriticalOvervoltage();
    printf("[PASS] Test 7: Critical overvoltage\n");

    Test_ADCFailure();
    printf("[PASS] Test 8: ADC failure\n");

    Test_FaultLatching();
    printf("[PASS] Test 9: Fault latching\n");

    Test_ManualReset();
    printf("[PASS] Test 10: Manual reset\n");

    printf("\nAll 10 protection tests passed!\n");

    return 0;
}
