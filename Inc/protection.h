
#ifndef PROTECTION_H
#define PROTECTION_H

#include <stdbool.h>
#include <stdint.h>

#include "critical_fault.h"

#define UNDERVOLTAGE_THRESHOLD 10.5f
#define OVERVOLTAGE_THRESHOLD 14.5f

#define RESET_MIN_VOLTAGE 11.0f
#define RESET_MAX_VOLTAGE 14.0f

#define FAULT_CONFIRMATION_MS 100U
#define STARTUP_CONFIRMATION_MS 100U

typedef enum
{
    PROTECTION_STARTUP,
    PROTECTION_NORMAL,
    PROTECTION_UNDERVOLTAGE,
    PROTECTION_OVERVOLTAGE,
    PROTECTION_SENSOR_FAULT,
    PROTECTION_CRITICAL_FAULT
} ProtectionState;

typedef struct
{
    ProtectionState state;
    bool load_enabled;

    uint32_t fault_timer_ms;
    uint32_t startup_timer_ms;

    ProtectionState pending_fault;

    /* Latched critical fault reason. */
    CriticalFault critical_reason;

} ProtectionSystem;

void Protection_Init(ProtectionSystem *system);

void Protection_Update(
    ProtectionSystem *system,
    float voltage,
    bool sensor_valid,
    bool reset_requested
);

void Protection_UpdateTimed(
    ProtectionSystem *system,
    float voltage,
    bool sensor_valid,
    bool reset_requested,
    uint32_t elapsed_ms
);

/* Immediately latch a detected critical fault. */
void Protection_TripCritical(
    ProtectionSystem *system,
    CriticalFault fault
);

#endif
