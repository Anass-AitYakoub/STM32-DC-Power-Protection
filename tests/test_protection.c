
/**
 * @file test_protection.c
 * @brief Unit tests for the STM32 DC Power Protection System.
 *
 * These tests run on a Windows PC without STM32 hardware.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "protection.h"

/* Test 1: Verify safe startup state. */
static void test_startup(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    assert(system.state == PROTECTION_STARTUP);
    assert(system.load_enabled == false);
}

/* Test 2: Verify normal voltage operation. */
static void test_normal_voltage(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    Protection_Update(&system, 12.0f, true, false);

    assert(system.state == PROTECTION_NORMAL);
    assert(system.load_enabled == true);
}

/* Test 3: Verify undervoltage detection. */
static void test_undervoltage(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    Protection_Update(&system, 9.0f, true, false);

    assert(system.state == PROTECTION_UNDERVOLTAGE);
    assert(system.load_enabled == false);
}

/* Test 4: Verify overvoltage detection. */
static void test_overvoltage(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    Protection_Update(&system, 16.0f, true, false);

    assert(system.state == PROTECTION_OVERVOLTAGE);
    assert(system.load_enabled == false);
}

/* Test 5: Verify that faults remain latched. */
static void test_fault_latching(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    Protection_Update(&system, 12.0f, true, false);
    Protection_Update(&system, 9.0f, true, false);
    Protection_Update(&system, 12.0f, true, false);

    assert(system.state == PROTECTION_UNDERVOLTAGE);
    assert(system.load_enabled == false);
}

/* Test 6: Verify manual reset after voltage recovery. */
static void test_manual_reset(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    Protection_Update(&system, 9.0f, true, false);
    Protection_Update(&system, 12.0f, true, true);

    assert(system.state == PROTECTION_NORMAL);
    assert(system.load_enabled == true);
}

/* Test 7: Reject reset while voltage is unsafe. */
static void test_invalid_reset(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    Protection_Update(&system, 16.0f, true, false);
    Protection_Update(&system, 16.0f, true, true);

    assert(system.state == PROTECTION_OVERVOLTAGE);
    assert(system.load_enabled == false);
}

/* Test 8: Verify invalid sensor detection. */
static void test_sensor_fault(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    Protection_Update(&system, 12.0f, false, false);

    assert(system.state == PROTECTION_SENSOR_FAULT);
    assert(system.load_enabled == false);
}

/* Test 9: Verify that sensor faults remain latched. */
static void test_sensor_fault_latching(void)
{
    ProtectionSystem system;
    Protection_Init(&system);

    Protection_Update(&system, 12.0f, false, false);
    Protection_Update(&system, 12.0f, true, false);

    assert(system.state == PROTECTION_SENSOR_FAULT);
    assert(system.load_enabled == false);
}

/* Run all unit tests. */
int main(void)
{
    test_startup();
    test_normal_voltage();
    test_undervoltage();
    test_overvoltage();
    test_fault_latching();
    test_manual_reset();
    test_invalid_reset();
    test_sensor_fault();
    test_sensor_fault_latching();

    printf("All 9 Embedded C protection tests passed.\n");

    return 0;
}
