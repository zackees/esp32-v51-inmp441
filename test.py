import os

HERE = os.path.dirname(__file__)

DEFS_FILE = os.path.join(HERE, "src", "defs.h")


def read_utf8(file: str) -> str:
    with open(file, "r", encoding="utf-8") as f:
        return f.read()


DEFS_CONTENT = read_utf8(DEFS_FILE)

DEFS_LINES = DEFS_CONTENT.splitlines()

for line in DEFS_LINES:
    if line.strip().startswith("#define DEBUG"):
        raise RuntimeError("DEBUG macro found in defs.h")

print("All good!")
