
"""
UART telemetry simulator for the
STM32 Smart DC Power Protection System.

This simulator generates telemetry strings using
the same format as the embedded STM32 firmware.

No physical serial port is required.
"""

import random
import time

from serial_protocol import parse_serial_data


class UARTSimulator:
    def __init__(self):
        self.sample_count = 0

    def generate_message(self):
        """Generate one simulated UART telemetry message."""

        cycle = self.sample_count % 100
        self.sample_count += 1

        if cycle < 25:
            voltage = 12.0 + random.uniform(-0.1, 0.1)
            state = "NORMAL"
            fault = "NONE"

        elif cycle < 40:
            voltage = 10.0 + random.uniform(-0.1, 0.1)
            state = "UNDERVOLTAGE"
            fault = "NONE"

        elif cycle < 55:
            voltage = 12.0 + random.uniform(-0.1, 0.1)
            state = "UNDERVOLTAGE"
            fault = "NONE"

        elif cycle < 75:
            voltage = 17.0 + random.uniform(-0.1, 0.1)
            state = "CRITICAL_FAULT"
            fault = "OVERVOLTAGE"

        elif cycle < 90:
            voltage = 12.0 + random.uniform(-0.1, 0.1)
            state = "CRITICAL_FAULT"
            fault = "OVERVOLTAGE"

        elif cycle < 95:
            voltage = 12.0
            state = "STARTUP"
            fault = "NONE"

        else:
            voltage = 12.0
            state = "NORMAL"
            fault = "NONE"

        # Physical load control remains disabled.
        load = "OFF"

        return (
            f"DATA,{voltage:.2f},"
            f"{state},{load},{fault}"
        )

    def read_data(self):
        """Simulate receiving and parsing one UART message."""

        message = self.generate_message()

        return parse_serial_data(message)


def main():
    simulator = UARTSimulator()

    print("STM32 UART Telemetry Simulator")
    print("Press Ctrl+C to stop.\n")

    try:
        while True:
            message = simulator.generate_message()
            print(message)

            time.sleep(0.5)

    except KeyboardInterrupt:
        print("\nSimulator stopped.")


if __name__ == "__main__":
    main()
