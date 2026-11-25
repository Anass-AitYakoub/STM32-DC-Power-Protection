
"""
Serial reader for the STM32 DC Power Protection System.

Receives and validates telemetry messages sent by
the STM32 through a USB virtual COM port.
"""

import serial
from serial.tools import list_ports

from serial_protocol import parse_serial_data


class STM32SerialReader:
    def __init__(self, port: str, baudrate: int = 115200):
        self.port = port
        self.baudrate = baudrate
        self.connection = None

    def connect(self):
        """Open the serial connection."""

        if self.connection is not None:
            return

        self.connection = serial.Serial(
            port=self.port,
            baudrate=self.baudrate,
            timeout=0.1
        )

        print(f"Connected to STM32 on {self.port}")

    def read_data(self):
        """
        Read and validate one telemetry message.

        Returns ProtectionData when a valid message
        is received, otherwise None.
        """

        if self.connection is None:
            return None

        raw = self.connection.readline()

        if not raw:
            return None

        try:
            line = raw.decode("ascii").strip()
            return parse_serial_data(line)

        except (UnicodeDecodeError, ValueError) as error:
            print(f"Invalid serial data: {error}")
            return None

    def disconnect(self):
        """Close the serial connection."""

        if self.connection is not None:
            self.connection.close()
            self.connection = None

            print("Serial connection closed")


def show_available_ports():
    """List available serial ports."""

    ports = list(list_ports.comports())

    if not ports:
        print("No serial ports detected")
        return

    print("Available serial ports:")

    for port in ports:
        print(f"  {port.device}: {port.description}")


if __name__ == "__main__":
    show_available_ports()
