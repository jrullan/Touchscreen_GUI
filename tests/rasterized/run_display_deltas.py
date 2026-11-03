#!/usr/bin/env python3
"""Isolated Display host/real-GFX tests and actual base-source measurements."""
from pathlib import Path
import argparse
import hashlib
import io
import os
import subprocess
import tarfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--host', action='store_true', help='operation/ownership/allocation suite')
parser.add_argument('--baseline', action='store_true', help='measure exact f4a8f9c sources (raster only)')
parser.add_argument('--measure', action='store_true', help='print one-cell raster write count only')
args = parser.parse_args()
if args.host and (args.baseline or args.measure):
    parser.error('--baseline/--measure are raster-only')
BUILD = Path(os.environ.get('TMPDIR', str(ROOT / '.host-build'))) / 'gui-display-deltas'
BUILD.mkdir(parents=True, exist_ok=True)
source_root = ROOT
if args.baseline:
    source_root = BUILD / 'base-f4a8f9c'
    source_root.mkdir(exist_ok=True)
    archive = subprocess.check_output(['git', 'archive', 'f4a8f9c'], cwd=ROOT)
    # Exact complete top-level production headers and sources, no worktree edits.
    with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
        for member in tar.getmembers():
            if member.isfile() and '/' not in member.name and Path(member.name).suffix in {'.h', '.cpp'}:
                stream = tar.extractfile(member)
                assert stream is not None
                (source_root / member.name).write_bytes(stream.read())
excluded = {'Adafruit_FT6206.cpp', 'Adafruit_STMPE610.cpp', 'Seeedtouch.cpp', 'XPT2046_Touchscreen.cpp'}
sources = [str(p) for p in sorted(source_root.glob('*.cpp')) if p.name not in excluded]
command = ['g++', '-std=c++17', '-g', '-O1', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie',
           '-fsanitize=address,undefined', '-Wno-write-strings', '-Wno-conversion-null', '-Wno-return-type',
           '-DARDUINO=10819', '-DESP8266']
if args.host:
    command += ['-DHOST_HEAP_MEASURE', '-I'+str(ROOT/'tests/host/stubs'),
                '-I'+str(source_root), str(ROOT/'tests/host/test_display_deltas.cpp'),
                '-Wl,--wrap=free,--wrap=_Znwm,--wrap=_ZdlPv,--wrap=_ZdlPvm']
else:
    GFX = Path(os.environ.get('ADAFRUIT_GFX_DIR', str(Path.home()/'Arduino/libraries/Adafruit_GFX_Library')))
    for name in ('Adafruit_GFX.cpp', 'Adafruit_GFX.h', 'glcdfont.c', 'gfxfont.h'):
        if not (GFX/name).is_file():
            raise SystemExit('SETUP REQUIRED: install Adafruit GFX or set ADAFRUIT_GFX_DIR; missing '+name)
        print(name, 'sha256='+hashlib.sha256((GFX/name).read_bytes()).hexdigest(), flush=True)
    if args.baseline:
        command += ['-DDISPLAY_DELTAS_BASELINE']
    command += ['-I'+str(ROOT/'tests/rasterized/display_deltas_stubs'),
                '-I'+str(ROOT/'tests/rasterized/stubs'), '-I'+str(GFX), '-I'+str(source_root),
                str(ROOT/'tests/rasterized/test_display_deltas.cpp'), str(GFX/'Adafruit_GFX.cpp')]
command += [str(ROOT/'tests/host/allocation_hooks.cpp'), '-Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc']
binary = BUILD / ('host' if args.host else 'baseline' if args.baseline else 'raster')
print('BUILD', binary.name, flush=True)
subprocess.run(command + sources + ['-o', str(binary)], check=True)
subprocess.run([str(binary)] + (['measure'] if args.baseline or args.measure else []), check=True,
               env=dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
                        UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
