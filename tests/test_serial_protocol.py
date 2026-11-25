
"""
Unit tests for STM32 UART telemetry protocol.
"""

import sys
import unittest
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "python"))

from serial_protocol import parse_serial_data
from uart_simulator import UARTSimulator


class TestSerialProtocol(unittest.TestCase):

    def test_normal_message(self):
        data = parse_serial_data(
            "DATA,12.04,NORMAL,OFF,NONE"
        )

        self.assertAlmostEqual(data.voltage, 12.04)
        self.assertEqual(data.state, "NORMAL")
        self.assertFalse(data.load_enabled)

    def test_critical_fault_message(self):
        data = parse_serial_data(
            "DATA,17.10,CRITICAL_FAULT,OFF,OVERVOLTAGE"
        )

        self.assertEqual(data.state, "CRITICAL_FAULT")
        self.assertEqual(
            data.critical_fault,
            "OVERVOLTAGE"
        )

    def test_invalid_prefix(self):
        with self.assertRaises(ValueError):
            parse_serial_data(
                "INVALID,12.0,NORMAL,OFF,NONE"
            )

    def test_invalid_voltage(self):
        with self.assertRaises(ValueError):
            parse_serial_data(
                "DATA,abc,NORMAL,OFF,NONE"
            )

    def test_invalid_state(self):
        with self.assertRaises(ValueError):
            parse_serial_data(
                "DATA,12.0,UNKNOWN,OFF,NONE"
            )

    def test_invalid_load_status(self):
        with self.assertRaises(ValueError):
            parse_serial_data(
                "DATA,12.0,NORMAL,INVALID,NONE"
            )

    def test_invalid_fault_type(self):
        with self.assertRaises(ValueError):
            parse_serial_data(
                "DATA,12.0,CRITICAL_FAULT,OFF,UNKNOWN"
            )

    def test_missing_fields(self):
        with self.assertRaises(ValueError):
            parse_serial_data(
                "DATA,12.0,NORMAL"
            )

    def test_voltage_out_of_range(self):
        with self.assertRaises(ValueError):
            parse_serial_data(
                "DATA,30.0,NORMAL,OFF,NONE"
            )

    def test_simulator_message(self):
        simulator = UARTSimulator()

        data = simulator.read_data()

        self.assertEqual(data.state, "NORMAL")
        self.assertFalse(data.load_enabled)


if __name__ == "__main__":
    unittest.main()
