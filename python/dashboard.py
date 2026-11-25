
"""
STM32 Smart DC Power Protection System
Real-Time Monitoring Dashboard

Features:
- Simulation and Serial modes
- Real-time voltage monitoring
- Protection state and load status
- Critical fault alerts
- CSV data logging
"""

import argparse
import csv
import random
import time
from collections import deque
from datetime import datetime
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

from serial_protocol import ProtectionData
from serial_reader import STM32SerialReader
from uart_simulator import UARTSimulator

# --------------------------------------------------
# Command-line configuration
# --------------------------------------------------

parser = argparse.ArgumentParser(
    description="STM32 DC Power Protection Dashboard"
)

parser.add_argument(
    "--mode",
    choices=["simulation", "uart-simulator", "serial"],
    default="simulation",
    help="Dashboard operating mode"
)

parser.add_argument(
    "--port",
    default="COM4",
    help="Serial port used by the STM32"
)

parser.add_argument(
    "--baudrate",
    type=int,
    default=115200,
    help="UART communication speed"
)

args = parser.parse_args()


# --------------------------------------------------
# Dashboard configuration
# --------------------------------------------------

UPDATE_INTERVAL_MS = 500
MAX_DATA_POINTS = 100

UNDERVOLTAGE_LIMIT = 10.5
OVERVOLTAGE_LIMIT = 14.5

timestamps = deque(maxlen=MAX_DATA_POINTS)
voltages = deque(maxlen=MAX_DATA_POINTS)

start_time = time.monotonic()
sample_index = 0

serial_reader = None
uart_simulator = UARTSimulator()

# --------------------------------------------------
# CSV logging
# --------------------------------------------------

PROJECT_ROOT = Path(__file__).resolve().parent.parent
LOG_DIRECTORY = PROJECT_ROOT / "logs"

LOG_DIRECTORY.mkdir(parents=True, exist_ok=True)

log_file = LOG_DIRECTORY / (
    "voltage_log_"
    + datetime.now().strftime("%Y%m%d_%H%M%S")
    + ".csv"
)

with log_file.open(
    "w",
    newline="",
    encoding="utf-8"
) as file:
    writer = csv.writer(file)

    writer.writerow([
        "timestamp",
        "voltage",
        "protection_state",
        "load_status",
        "critical_fault"
    ])


def log_measurement(data):
    """Save one validated measurement to CSV."""

    with log_file.open(
        "a",
        newline="",
        encoding="utf-8"
    ) as file:

        writer = csv.writer(file)

        writer.writerow([
            datetime.now().isoformat(
                timespec="milliseconds"
            ),
            f"{data.voltage:.3f}",
            data.state,
            "ON" if data.load_enabled else "OFF",
            data.critical_fault
        ])


# --------------------------------------------------
# Simulation mode
# --------------------------------------------------

def generate_simulated_data():
    """Generate a repeating protection scenario."""

    global sample_index

    cycle = sample_index % 100
    sample_index += 1

    if cycle < 25:
        voltage = 12.0 + random.uniform(-0.15, 0.15)
        state = "NORMAL"
        load_enabled = True
        fault = "NONE"

    elif cycle < 40:
        voltage = 10.0 + random.uniform(-0.1, 0.1)
        state = "UNDERVOLTAGE"
        load_enabled = False
        fault = "NONE"

    elif cycle < 55:
        voltage = 12.0 + random.uniform(-0.1, 0.1)
        state = "UNDERVOLTAGE"
        load_enabled = False
        fault = "NONE"

    elif cycle < 75:
        voltage = 17.0 + random.uniform(-0.1, 0.1)
        state = "CRITICAL_FAULT"
        load_enabled = False
        fault = "OVERVOLTAGE"

    elif cycle < 90:
        voltage = 12.0 + random.uniform(-0.1, 0.1)
        state = "CRITICAL_FAULT"
        load_enabled = False
        fault = "OVERVOLTAGE"

    elif cycle < 95:
        voltage = 12.0 + random.uniform(-0.1, 0.1)
        state = "STARTUP"
        load_enabled = False
        fault = "NONE"

    else:
        voltage = 12.0 + random.uniform(-0.1, 0.1)
        state = "NORMAL"
        load_enabled = True
        fault = "NONE"

    return ProtectionData(
        voltage=voltage,
        state=state,
        load_enabled=load_enabled,
        critical_fault=fault
    )


# --------------------------------------------------
# Serial communication
# --------------------------------------------------

def connect_serial():
    """Connect to the STM32 serial interface."""

    global serial_reader

    if args.mode != "serial":
        return

    serial_reader = STM32SerialReader(
        port=args.port,
        baudrate=args.baudrate
    )

    try:
        serial_reader.connect()

    except Exception as error:
        print(f"Serial connection failed: {error}")
        raise SystemExit(1) from error



def receive_data():
    """Get one measurement from the selected mode."""

    if args.mode == "simulation":
        return generate_simulated_data()

    if args.mode == "uart-simulator":
        return uart_simulator.read_data()

    if args.mode == "serial":
        if serial_reader is None:
            return None

        return serial_reader.read_data()

    return None


# --------------------------------------------------
# Create dashboard
# --------------------------------------------------

fig, ax = plt.subplots(figsize=(12, 7))

fig.canvas.manager.set_window_title(
    "STM32 DC Power Protection - Dashboard"
)

fig.subplots_adjust(
    top=0.72,
    bottom=0.13,
    left=0.09,
    right=0.95
)

fig.suptitle(
    "STM32 SMART DC POWER PROTECTION SYSTEM",
    fontsize=16,
    fontweight="bold",
    y=0.97
)

mode_text = fig.text(
    0.09,
    0.92,
    f"MODE: {args.mode.upper()}",
    fontsize=10
)

voltage_text = fig.text(
    0.10,
    0.84,
    "Voltage: -- V",
    fontsize=16,
    fontweight="bold"
)

state_text = fig.text(
    0.44,
    0.84,
    "State: STARTUP",
    fontsize=12
)

load_text = fig.text(
    0.80,
    0.84,
    "Load: OFF",
    fontsize=13,
    fontweight="bold"
)

alert_text = fig.text(
    0.10,
    0.77,
    "Waiting for measurements...",
    fontsize=12
)

line, = ax.plot(
    [],
    [],
    linewidth=2,
    label="Input Voltage"
)

ax.axhline(
    UNDERVOLTAGE_LIMIT,
    color="orange",
    linestyle="--",
    label="Undervoltage threshold"
)

ax.axhline(
    OVERVOLTAGE_LIMIT,
    color="red",
    linestyle="--",
    label="Overvoltage threshold"
)

ax.set_title(
    "Real-Time Voltage Monitoring"
)

ax.set_xlabel("Time (s)")
ax.set_ylabel("Voltage (V)")

ax.set_ylim(0, 24)
ax.set_xlim(0, 50)

ax.grid(True, alpha=0.3)
ax.legend(loc="upper right")


# --------------------------------------------------
# Dashboard updates
# --------------------------------------------------

def update_dashboard(_frame):
    """Refresh measurements, indicators and graph."""

    try:
        data = receive_data()

    except (OSError, ValueError) as error:
        print(f"Data reception error: {error}")

        alert_text.set_text(
            "SERIAL COMMUNICATION ERROR"
        )

        alert_text.set_color("red")
        return ()

    # No new serial message available.
    if data is None:
        return ()

    elapsed = time.monotonic() - start_time

    timestamps.append(elapsed)
    voltages.append(data.voltage)

    # Record validated measurements.
    log_measurement(data)

    # Update graph.
    line.set_data(
        list(timestamps),
        list(voltages)
    )

    left = max(0.0, elapsed - 50.0)
    right = max(50.0, elapsed + 1.0)

    ax.set_xlim(left, right)

    # Update voltage display.
    voltage_text.set_text(
        f"Voltage: {data.voltage:.2f} V"
    )

    # Update protection state.
    state_text.set_text(
        f"State: {data.state}"
    )

    # Update logical load status.
    load_status = (
        "ON" if data.load_enabled else "OFF"
    )

    load_text.set_text(
        f"Load: {load_status}"
    )

    # Apply status colors and alerts.
    if data.state == "NORMAL":
        state_text.set_color("green")

        alert_text.set_color("green")
        alert_text.set_text(
            "System operating normally"
        )

    elif data.state == "CRITICAL_FAULT":
        state_text.set_color("red")

        alert_text.set_color("red")
        alert_text.set_text(
            f"CRITICAL ALERT: {data.critical_fault}"
        )

    elif data.state == "STARTUP":
        state_text.set_color("gray")

        alert_text.set_color("gray")
        alert_text.set_text(
            "Startup validation in progress"
        )

    else:
        state_text.set_color("darkorange")

        alert_text.set_color("darkorange")
        alert_text.set_text(
            f"PROTECTION ALERT: {data.state}"
        )

    load_text.set_color(
        "green" if data.load_enabled else "red"
    )

    return (
        line,
        voltage_text,
        state_text,
        load_text,
        alert_text
    )


# --------------------------------------------------
# Cleanup
# --------------------------------------------------

def on_close(_event):
    """Close the serial connection when exiting."""

    if serial_reader is not None:
        serial_reader.disconnect()


fig.canvas.mpl_connect(
    "close_event",
    on_close
)


# --------------------------------------------------
# Application entry point
# --------------------------------------------------

def main():
    """Start the monitoring application."""

    connect_serial()

    print("======================================")
    print(" STM32 DC POWER PROTECTION DASHBOARD")
    print("======================================")
    print(f"Mode: {args.mode.upper()}")

    if args.mode == "serial":
        print(f"Serial port: {args.port}")
        print(f"Baud rate: {args.baudrate}")

    print(f"CSV log: {log_file}")

    animation = FuncAnimation(
        fig,
        update_dashboard,
        interval=UPDATE_INTERVAL_MS,
        blit=False,
        cache_frame_data=False
    )

    # Keep a reference to the animation alive.
    fig._dashboard_animation = animation

    try:
        plt.show()

    finally:
        on_close(None)


if __name__ == "__main__":
    main()
