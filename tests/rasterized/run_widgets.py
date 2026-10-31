#!/usr/bin/env python3
"""Real-GFX widget pixels and nested transaction tests, with sanitizers."""
from pathlib import Path
import os
import subprocess

ROOT = Path(__file__).resolve().parents[2]
GFX = Path(os.environ.get('ADAFRUIT_GFX_DIR', str(Path.home() / 'Arduino/libraries/Adafruit_GFX_Library')))
for name in ('Adafruit_GFX.cpp', 'Adafruit_GFX.h', 'glcdfont.c', 'gfxfont.h'):
    if not (GFX / name).is_file():
        raise SystemExit('SETUP REQUIRED: install Adafruit GFX or set ADAFRUIT_GFX_DIR')
BUILD = Path(os.environ.get('TMPDIR', str(ROOT / '.host-build'))) / 'gui-widget-raster'
BUILD.mkdir(parents=True, exist_ok=True)
excluded = {'Adafruit_FT6206.cpp', 'Adafruit_STMPE610.cpp', 'Seeedtouch.cpp', 'XPT2046_Touchscreen.cpp'}
sources = [str(p) for p in sorted(ROOT.glob('*.cpp')) if p.name not in excluded]
for widget, macro in [('bars', 'BAR'), ('terminal', 'TERMINAL')]:
    binary = BUILD / widget
    command = ['g++', '-std=c++17', '-g', '-O1', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie',
               '-fsanitize=address,undefined', '-Wno-write-strings', '-Wno-conversion-null', '-Wno-return-type',
               '-DARDUINO=10819', '-DESP8266', '-D' + macro + '_REAL_GFX', '-D' + macro + '_TRANSACTIONS',
               '-I' + str(ROOT / 'tests/transactions/stubs'), '-I' + str(ROOT / 'tests/rasterized/stubs'),
               '-I' + str(GFX), '-I' + str(ROOT), str(ROOT / ('tests/host/test_render_' + widget + '.cpp')),
               str(GFX / 'Adafruit_GFX.cpp'), *sources, '-o', str(binary)]
    print('BUILD REAL GFX', widget, flush=True)
    subprocess.run(command, check=True)
    subprocess.run([str(binary)], check=True, env=dict(os.environ,
        ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
