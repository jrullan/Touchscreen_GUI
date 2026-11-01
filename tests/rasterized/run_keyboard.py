#!/usr/bin/env python3
"""Compile actual Keyboard/core sources and installed Adafruit GFX under sanitizers."""
from pathlib import Path
import hashlib
import os
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
GFX = Path(os.environ.get('ADAFRUIT_GFX_DIR', str(Path.home() / 'Arduino/libraries/Adafruit_GFX_Library')))
REQUIRED = ('Adafruit_GFX.cpp', 'Adafruit_GFX.h', 'glcdfont.c', 'gfxfont.h', 'Fonts/FreeSans18pt7b.h')
for name in REQUIRED:
    if not (GFX / name).is_file():
        sys.exit('SETUP REQUIRED: install Adafruit GFX or set ADAFRUIT_GFX_DIR; missing ' + name)
CASES = ('labels', 'restore', 'edges', 'modes', 'customfont', 'font0')
cases = sys.argv[1:] or CASES
if any(case not in CASES for case in cases):
    sys.exit('usage: python3 tests/rasterized/run_keyboard.py [' + '|'.join(CASES) + ' ...]')
build = Path(os.environ.get('TMPDIR', str(ROOT / '.host-build'))) / 'gui-keyboard-raster'
build.mkdir(parents=True, exist_ok=True)
excluded = {'Adafruit_FT6206.cpp', 'Adafruit_STMPE610.cpp', 'Seeedtouch.cpp', 'XPT2046_Touchscreen.cpp'}
sources = [str(p) for p in sorted(ROOT.glob('*.cpp')) if p.name not in excluded]
# Never add host/stubs: its fake Adafruit_GFX.h would replace the rasterizer.
command = ['g++', '-std=c++17', '-g', '-O1', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie',
           '-fsanitize=address,undefined', '-Wno-write-strings', '-Wno-conversion-null', '-Wno-return-type',
           '-DARDUINO=10819', '-DESP8266', '-I' + str(ROOT / 'tests/transactions/stubs'),
           '-I' + str(ROOT / 'tests/rasterized/stubs'), '-I' + str(GFX), '-I' + str(ROOT),
           str(ROOT / 'tests/rasterized/test_keyboard.cpp'), str(GFX / 'Adafruit_GFX.cpp'),
           *sources, '-o', str(build / 'keyboard')]
print('GFX SOURCE:', GFX, flush=True)
for name in ('Adafruit_GFX.cpp', 'glcdfont.c'):
    print(name, 'sha256=' + hashlib.sha256((GFX / name).read_bytes()).hexdigest(), flush=True)
subprocess.run(command, check=True)
env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
           UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
failed = False
for case in cases:
    result = subprocess.run([str(build / 'keyboard'), case], env=env)
    print(case + ': ' + ('PASS' if result.returncode == 0 else 'FAIL'), flush=True)
    failed |= result.returncode != 0
sys.exit(int(failed))
