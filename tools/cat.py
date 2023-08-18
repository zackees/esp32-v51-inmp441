"""Read data from a serial port and write it to a file."""

# pylint: disable=invalid-name

import sys

sys.path.append(".")
from base import OpenSerial, SerialPort, query_comport


def cat(serial_port: SerialPort, baudrate: int, timeout: int, outstream=None) -> None:
    """Read data from serial port and write it to a file."""
    # Connect to serial port
    outstream = outstream or sys.stdout
    with OpenSerial(serial_port.port, baudrate, timeout) as ser:
        sys.stderr.write(f"Connected to {ser.name} at {ser.baudrate} baud\n")
        # Open the output file
        while True:
            try:
                line = ser.readline()  # read a line from the serial port
                if line:
                    line = line.decode()  # convert bytes to string
                    outstream.write(f"{line}")
            except KeyboardInterrupt:
                print("Interrupted!")
                break


def main() -> int:
    """Main function."""
    BAUDRATE = 115200
    port = query_comport()
    cat(serial_port=port, baudrate=BAUDRATE, timeout=5)
    return 0


if __name__ == "__main__":
    sys.exit(main())
