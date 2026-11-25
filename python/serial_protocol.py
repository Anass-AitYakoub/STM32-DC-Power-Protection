
"""
Serial communication protocol for the
STM32 Smart DC Power Protection System.
"""

from dataclasses import dataclass
import math


VALID_STATES = {
    "STARTUP",
    "NORMAL",
    "UNDERVOLTAGE",
    "OVERVOLTAGE",
    "SENSOR_FAULT",
    "CRITICAL_FAULT",
}

VALID_FAULTS = {
    "NONE",
    "ADC_FAILURE",
    "INVALID_READING",
    "UNDERVOLTAGE",
    "OVERVOLTAGE",
}


@dataclass
class ProtectionData:
    voltage: float
    state: str
    load_enabled: bool
    critical_fault: str


def parse_serial_data(line: str) -> ProtectionData:
    """Parse and validate one STM32 serial message."""

    parts = line.strip().split(",")

    if len(parts) != 5:
        raise ValueError("Invalid number of fields")

    if parts[0] != "DATA":
        raise ValueError("Invalid message prefix")

    try:
        voltage = float(parts[1])
    except ValueError as error:
        raise ValueError("Invalid voltage value") from error

    if not math.isfinite(voltage) or not 0.0 <= voltage <= 24.0:
        raise ValueError("Voltage out of supported range")

    state = parts[2]

    if state not in VALID_STATES:
        raise ValueError("Invalid protection state")

    if parts[3] not in {"ON", "OFF"}:
        raise ValueError("Invalid load status")

    fault = parts[4]

    if fault not in VALID_FAULTS:
        raise ValueError("Invalid critical fault type")

    return ProtectionData(
        voltage=voltage,
        state=state,
        load_enabled=(parts[3] == "ON"),
        critical_fault=fault,
    )
