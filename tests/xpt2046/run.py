#!/usr/bin/env python3
"""Isolated actual bundled XPT + adapter + mapper; host only, no board access."""
from pathlib import Path
import os
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent

def main():
    compiler = shutil.which("g++")
    if not compiler:
        raise SystemExit("SETUP REQUIRED: g++ with ASan/UBSan is missing")
    sources = [HERE / "test_xpt2046.cpp"] + [ROOT / name for name in (
        "XPT2046_Touchscreen.cpp", "TouchMapper.cpp", "Point.cpp", "Touch.cpp")]
    for path in sources + [ROOT / "tests/host/stubs/Arduino.h"]:
        if not path.is_file():
            raise SystemExit(f"SETUP REQUIRED: missing source/stub {path}")
    modes = sys.argv[1:] or ["irq", "corners", "cache"]
    scratch = Path(os.environ.get("TMPDIR", str(Path.home() / ".hermes/profiles/uncensored/cache/scratch")))
    scratch.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="touch-xpt2046-", dir=scratch) as build:
        binary = Path(build) / "test_xpt2046"
        command = [compiler, "-std=c++17", "-g", "-O1", "-fno-omit-frame-pointer",
                   "-fno-pie", "-no-pie", "-fsanitize=address,undefined",
                   "-DARDUINO=10819", "-I" + str(HERE / "stubs"), "-I" + str(ROOT),
                   *map(str, sources), "-o", str(binary)]
        print("BUILD actual bundled XPT2046 + mapper + Point + Touch", flush=True)
        subprocess.run(command, check=True)
        env = dict(os.environ, ASAN_OPTIONS="detect_leaks=1:halt_on_error=1",
                   UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1")
        for mode in modes:
            print("RUN", mode, flush=True)
            subprocess.run([str(binary), mode], check=True, env=env)

if __name__ == "__main__":
    main()
