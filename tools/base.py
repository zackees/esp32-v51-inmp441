"""Read data from a serial port and write it to a file."""

# pylint: disable=invalid-name
# pylint: disable=wrong-import-position,missing-function-docstring,missing-class-docstring

import sys
from dataclasses import dataclass

import serial  # type: ignore
import serial.tools.list_ports  # type: ignore


@dataclass
class SerialPort:
    """Serial port data."""

    port: str
    description: str
    hardware_id: str

    def __repr__(self) -> str:
        return f"{self.port}: {self.description} [{self.hardware_id}]"


# use with a with statement
class OpenSerial:
    def __init__(self, port: str, baudrate: int, timeout: int):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser = None

    def __enter__(self):
        self.ser = serial.Serial(self.port, self.baudrate, timeout=self.timeout)
        return self.ser

    def __exit__(self, exc_type, exc_value, traceback):
        self.ser.close()

    def readline(self) -> str | None:
        try:
          return self.ser.readline()
        except serial.SerialException:
          return None

def get_comports() -> list[SerialPort]:
    """List serial ports."""
    ports = serial.tools.list_ports.comports()
    out = [SerialPort(port, desc, hwid) for port, desc, hwid in sorted(ports)]
    return out


def select_comport(ports: list[SerialPort]) -> SerialPort:
    """Select a serial port."""
    if len(ports) == 1:
        return ports[0]
    for i, port in enumerate(ports):
        if "usbmodem" in port.port:
            return port
    for i, port in enumerate(ports):
        print(f"{i}: {port}")
    while True:
        try:
            selection = int(input("Select a serial port: "))
            if selection < 0 or selection >= len(ports):
                print("Invalid selection")
                continue
            return ports[selection]
        except ValueError:
            print("Invalid selection")


def query_comport() -> SerialPort:
    """Query a serial port."""
    ports = get_comports()
    if len(ports) == 0:
        print("No COM ports found")
        sys.exit(1)
    return select_comport(ports)
