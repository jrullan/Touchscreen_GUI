#!/usr/bin/env python3
"""Actual IconButton + installed real GFX; isolated instrumented panel/flash seam."""
from pathlib import Path
import hashlib
import os
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
GFX = Path(os.environ.get('ADAFRUIT_GFX_DIR', str(Path.home() / 'Arduino/libraries/Adafruit_GFX_Library')))
for name in ('Adafruit_GFX.cpp', 'Adafruit_GFX.h', 'glcdfont.c', 'gfxfont.h'):
    if not (GFX / name).is_file():
        raise SystemExit('SETUP REQUIRED: install Adafruit GFX or set ADAFRUIT_GFX_DIR; missing ' + name)
BUILD = Path(os.environ.get('TMPDIR', str(ROOT / '.host-build'))) / 'gui-icon-runs-raster'
BUILD.mkdir(parents=True, exist_ok=True)
excluded = {'Adafruit_FT6206.cpp', 'Adafruit_STMPE610.cpp', 'Seeedtouch.cpp', 'XPT2046_Touchscreen.cpp'}
sources = [str(p) for p in sorted(ROOT.glob('*.cpp')) if p.name not in excluded]
if sys.argv[1:] not in ([], ['--baseline']):
    raise SystemExit('usage: run_icon_runs.py [--baseline]')
baseline = bool(sys.argv[1:])
if baseline:
    # Execute the actual pre-optimization production implementation, not a
    # fabricated operation estimate. Only safe in-bounds asset cases run here.
    source = subprocess.check_output(['git', 'show', '58eabc4:IconButton.cpp'], cwd=ROOT)
    baseline_source = BUILD / 'baseline_IconButton.cpp'
    baseline_source.write_bytes(source)
    sources = [p for p in sources if Path(p).name != 'IconButton.cpp'] + [str(baseline_source)]
    print('BASELINE IconButton.cpp 58eabc4 sha256=' + hashlib.sha256(source).hexdigest(), flush=True)
command = ['g++', '-std=c++17', '-g', '-O1', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie',
           '-fsanitize=address,undefined', '-Wno-write-strings', '-Wno-conversion-null', '-Wno-return-type',
           '-DARDUINO=10819', '-DESP8266', '-I' + str(ROOT / 'tests/rasterized/icon_stubs'),
           '-I' + str(ROOT / 'tests/rasterized/stubs'), '-I' + str(GFX), '-I' + str(ROOT),
           str(ROOT / 'tests/rasterized/test_icon_runs.cpp'), str(GFX / 'Adafruit_GFX.cpp'),
           *sources, '-o', str(BUILD / 'icon_runs')]
print('GFX SOURCE:', GFX, flush=True)
print('Adafruit_GFX.cpp sha256=' + hashlib.sha256((GFX / 'Adafruit_GFX.cpp').read_bytes()).hexdigest(), flush=True)
subprocess.run(command, check=True)
subprocess.run([str(BUILD / 'icon_runs'), *(['--baseline'] if baseline else [])], check=True, env=dict(os.environ,
    ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
