#!/usr/bin/env python3
"""Real installed Adafruit_GFX rasterizer; only Arduino/bus/panel are substituted."""
from pathlib import Path
import os
import hashlib
import subprocess
import sys
ROOT = Path(__file__).resolve().parents[2]
GFX = Path(os.environ.get("ADAFRUIT_GFX_DIR", str(Path.home()/"Arduino/libraries/Adafruit_GFX_Library")))
required = ("Adafruit_GFX.cpp", "Adafruit_GFX.h", "glcdfont.c", "gfxfont.h")
if any(not (GFX/name).is_file() for name in required):
    sys.exit("SETUP REQUIRED: install Adafruit GFX Library or set ADAFRUIT_GFX_DIR to its source directory (no raster test run).")
build = Path(os.environ.get("TMPDIR", str(ROOT/".host-build")))/"touchscreen-gui-rasterized"
build.mkdir(parents=True, exist_ok=True)
excluded = {"Adafruit_FT6206.cpp", "Adafruit_STMPE610.cpp", "Seeedtouch.cpp", "XPT2046_Touchscreen.cpp"}
sources = [str(p) for p in sorted(ROOT.glob("*.cpp")) if p.name not in excluded]
# Do NOT include host/stubs: its Adafruit_GFX.h would shadow the actual library.
cmd = ["g++", "-std=c++17", "-g", "-O1", "-fno-omit-frame-pointer", "-fno-pie", "-no-pie",
       "-fsanitize=address,undefined", "-Wno-write-strings", "-Wno-conversion-null", "-Wno-return-type",
       "-DARDUINO=10819", "-DESP8266", "-I"+str(ROOT/"tests/rasterized/stubs"),
       "-I"+str(GFX), "-I"+str(ROOT), str(ROOT/"tests/rasterized/test_display.cpp"),
       str(GFX/"Adafruit_GFX.cpp"), *sources, "-o", str(build/"display")]
cases = sys.argv[1:] or ["wrap", "font0"]
if any(case not in ("wrap", "font0") for case in cases):
    sys.exit("usage: python3 tests/rasterized/run.py [wrap|font0 ...]")
print("GFX SOURCE:", GFX, flush=True)
for name in ("Adafruit_GFX.cpp", "glcdfont.c"):
    print(name, "sha256="+hashlib.sha256((GFX/name).read_bytes()).hexdigest(), flush=True)
subprocess.run(cmd, check=True)
env = dict(os.environ, ASAN_OPTIONS="detect_leaks=1:halt_on_error=1", UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1")
failed = False
for case in cases:
    result = subprocess.run([str(build/"display"), case], env=env)
    failed |= result.returncode != 0
sys.exit(int(failed))
