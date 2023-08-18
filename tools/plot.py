# pylint: disable=wrong-import-position,missing-function-docstring,missing-class-docstring,line-too-long,c-extension-no-member,invalid-name,broad-exception-caught

import sys
import warnings
import threading
import queue

from PyQt6 import QtWidgets
from PyQt6 import QtCore

import pyqtgraph as pg

sys.path.append(".")
from base import OpenSerial, SerialPort, query_comport

MAX_SAMPLES = 100
BAUDRATE = 115200

LABELS1 = [
    "dB",
    "dB_Min",
    "dB_Max",
    "dB_Mid",
    "dB_WindowMin",
    "dB_WindowMax",
]

LABELS2 = [
    "led",
]

COLORS1 = ["r", "g", "b", "c", "m", "y"]
COLORS2 = ["w"]

app = QtWidgets.QApplication([])  # Make the application global

def intlist() -> list[int]:
    return [0 for _ in range(MAX_SAMPLES)]

class LivePlotter(QtWidgets.QMainWindow):
    def __init__(self, labels: list[str], colors: list[str], title: str, y_min: int, y_max: int, xlabel: str, ylabel: str):
        super().__init__()
        self.setWindowTitle(title)

        self.win = pg.GraphicsLayoutWidget(show=True)
        self.setCentralWidget(self.win)

        self.x_data: list[int] = [0 for _ in range(MAX_SAMPLES)]
        self.y_data: list[list[int]] = [intlist() for _ in range(len(labels))]

        self.plot_item = self.win.addPlot(title=title)
        self.plot_item.setRange(yRange=[y_min, y_max])
        self.plot_item.showGrid(x=True, y=True)
        self.plot_item.setLabel("left", ylabel)
        self.plot_item.setLabel("bottom", xlabel)

        self.legend = self.plot_item.addLegend()

        self.lines = []
        for i in range(len(labels)):
            line = self.plot_item.plot(
                self.x_data, self.y_data[i], pen=pg.mkPen(colors[i])
            )
            self.lines.append(line)
            self.legend.addItem(line, labels[i])

    def update(self, x_value: int, y_values: list[int]):
        self.x_data.append(x_value)
        for i, yval in enumerate(y_values):
            if i >= len(self.y_data):
                break
            self.y_data[i].append(yval)

        self.x_data = self.x_data[-MAX_SAMPLES:]
        self.y_data = [y[-MAX_SAMPLES:] for y in self.y_data]

        for i, line in enumerate(self.lines):
            line.setData(self.x_data, self.y_data[i])

    def draw(self):
        app.processEvents()  # Use the global app here

class BufferedReadPlot:
    def __init__(self, serial_port: SerialPort, baudrate: int, timeout: int) -> None:
        self.ser = OpenSerial(port=serial_port.port, baudrate=baudrate, timeout=timeout)
        self.buffer: queue.Queue = queue.Queue()
        self.dead = False
        self.thread = threading.Thread(target=self.read_and_buffer, daemon=True)
        self.open_handle = self.ser.__enter__()
        self.thread.start()

    def __del__(self):
        self.ser.__exit__(None, None, None)

    def close(self):
        self.dead = True
        self.thread.join()
        self.ser.__exit__(None, None, None)


    def read_and_buffer(self):
        counter = 0
        while not self.dead:
            try:
                line_serial = self.ser.readline()
                if not line_serial:
                    continue
                line_serial = line_serial.decode().strip()
                print(line_serial)
                data = line_serial.split(",")
                try:
                    data_ints = [int(x) for x in data]
                except ValueError:
                    continue
                self.buffer.put((counter, data_ints))
                counter += 1
            except KeyboardInterrupt:
                self.dead = True
                return
            except Exception as e:
                warnings.warn(f"Error parsing {line_serial} because of {e}")

    def get_data(self):
        while not self.buffer.empty():
            yield self.buffer.get()

def main() -> int:
    port = query_comport()
    sys.stderr.write(f"Connected to {port} at {BAUDRATE} baud\n")
    buffered_reader = BufferedReadPlot(serial_port=port, baudrate=BAUDRATE, timeout=5)

    plotter1 = LivePlotter(labels=LABELS1, colors=COLORS1, title="dB Plots", y_min=40, y_max=100, xlabel="samples", ylabel="dB")
    plotter2 = LivePlotter(labels=LABELS2, colors=COLORS2, title="LED Plot", y_min=0, y_max=255, xlabel="samples", ylabel="power")

    plotter1.move(100, 100)  # Position the first plot window
    plotter2.move(400, 400)  # Position the second plot window

    plotter1.show()
    plotter2.show()

    try:
        while True:
            for x_value, y_values in buffered_reader.get_data():
                plotter1.update(x_value, y_values[:-1])  # Exclude "led" from the first plot
                plotter2.update(x_value, [y_values[-1]])  # Include only "led" in the second plot
            plotter1.draw()
            plotter2.draw()
    except KeyboardInterrupt:
        sys.stderr.write("Interrupted\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
