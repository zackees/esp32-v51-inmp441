# pylint: disable=invalid-name,line-too-long

import argparse
import os
import sys

HERE = os.path.dirname(os.path.realpath(__file__))
DEBUG_PLATFORM_INI = os.path.join(HERE, "platformio.debug.ini")

if not "PLATFORMIO_PATH" in os.environ:
    print("Please run this in the pio environment")
    sys.exit(1)

parser = argparse.ArgumentParser(
    description="Uploads and runs the debug build of the esp32c3 firmware"
)
parser.add_argument(
    "--log",
    nargs="?",
    const=True,
    default=False,
    help="Prints the serial output to the specified log file, or to the console if no file is specified",
)
parser.add_argument(
    "--plot",
    action="store_true",
    default=False,
    help="Plots the serial output to the console",
)

args = parser.parse_args()

if args.plot and args.log:
    print("Cannot plot and log at the same time")
    sys.exit(1)

cmd_logging = None
if args.log is not False:
    if args.log is True:
        cmd_logging = f"python {os.path.join('tools', 'cat.py')}"
    else:
        print(f"Logging to file: {args.log}")
        cmd_logging = f"python {os.path.join('tools', 'cat.py')} > {args.log}"

here = os.path.dirname(os.path.realpath(__file__))
os.chdir(here)
cmd = f"pio run --project-conf {DEBUG_PLATFORM_INI} -t upload -e esp32c3_debug"
rtn = os.system(cmd)
if rtn != 0:
    sys.exit(rtn)
if args.plot is not False:
    cmd_plot = f"cd tools && . ./activate.sh && python plot.py"
    os.system(cmd_plot)
elif args.log is not False:
    if args.log is True:
        cmd_plot = f"python {os.path.join('tools', 'cat.py')}"
    else:
        cmd_plot = f"python {os.path.join('tools', 'cat.py')} > {args.log}"
    os.system(cmd_logging)
