#!/usr/bin/env python3
"""Retained real-GFX X-axis raster and caller-state regression (no hardware)."""
from pathlib import Path
import hashlib
import os
import subprocess

ROOT = Path(__file__).resolve().parents[2]
GFX = Path(os.environ.get('ADAFRUIT_GFX_DIR', str(Path.home() / 'Arduino/libraries/Adafruit_GFX_Library')))
required = ('Adafruit_GFX.cpp', 'Adafruit_GFX.h', 'glcdfont.c', 'gfxfont.h',
            'Fonts/FreeSans18pt7b.h', 'Fonts/FreeSans24pt7b.h')
for name in required:
    if not (GFX / name).is_file():
        raise SystemExit('SETUP REQUIRED: install Adafruit GFX or set ADAFRUIT_GFX_DIR; missing ' + name)
BUILD = Path(os.environ.get('TMPDIR', str(ROOT / '.host-build'))) / 'gui-xaxis-raster'
BUILD.mkdir(parents=True, exist_ok=True)
excluded = {'Adafruit_FT6206.cpp', 'Adafruit_STMPE610.cpp', 'Seeedtouch.cpp', 'XPT2046_Touchscreen.cpp'}
sources = [str(p) for p in sorted(ROOT.glob('*.cpp')) if p.name not in excluded]
command = ['g++', '-std=c++17', '-g', '-O1', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie',
           '-fsanitize=address,undefined', '-Wno-write-strings', '-Wno-conversion-null', '-Wno-return-type',
           '-DARDUINO=10819', '-DESP8266', '-I' + str(ROOT / 'tests/rasterized/stubs'),
           '-I' + str(GFX), '-I' + str(ROOT), str(ROOT / 'tests/rasterized/test_xaxis.cpp'),
           str(GFX / 'Adafruit_GFX.cpp'), *sources, '-o', str(BUILD / 'xaxis')]
print('GFX SOURCE:', GFX, flush=True)
for name in required:
    print(name, 'sha256=' + hashlib.sha256((GFX / name).read_bytes()).hexdigest(), flush=True)
subprocess.run(command, check=True)
failed = False
for chart in ('Trend', 'MultiTrend'):
    print('RUN', chart, flush=True)
    result = subprocess.run([str(BUILD / 'xaxis'), chart], env=dict(os.environ,
        ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
    failed |= result.returncode != 0
raise SystemExit(int(failed))
