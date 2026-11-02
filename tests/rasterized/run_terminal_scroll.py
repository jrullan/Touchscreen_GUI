#!/usr/bin/env python3
"""Isolated real-source scroll copy/heap and real Adafruit GFX pixel tests."""
from pathlib import Path
import hashlib
import os
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
BUILD = Path(os.environ.get('TMPDIR', str(ROOT / '.host-build'))) / 'gui-terminal-scroll'
BUILD.mkdir(parents=True, exist_ok=True)
host = '--host' in sys.argv
baseline = '--baseline' in sys.argv
GFX = Path(os.environ.get('ADAFRUIT_GFX_DIR', str(Path.home() / 'Arduino/libraries/Adafruit_GFX_Library')))
flags = ['g++', '-std=c++17', '-g', '-O1', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie',
         '-fsanitize=address,undefined', '-Wno-write-strings', '-Wno-conversion-null', '-Wno-return-type',
         '-DARDUINO=10819', '-DESP8266']
excluded = {'Adafruit_FT6206.cpp', 'Adafruit_STMPE610.cpp', 'Seeedtouch.cpp', 'XPT2046_Touchscreen.cpp'}
sources = [str(p) for p in sorted(ROOT.glob('*.cpp')) if p.name not in excluded]
if host:
    flags += ['-DHOST_HEAP_MEASURE', '-DTERMINAL_SCROLL_MEASURE', '-fno-builtin-memmove', '-fno-builtin-memcpy',
              '-I'+str(ROOT/'tests/host/stubs'), '-I'+str(ROOT),
              str(ROOT/'tests/host/test_terminal_scroll.cpp'), str(ROOT/'tests/host/allocation_hooks.cpp')]
    flags += ['-Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free,--wrap=_Znwm,--wrap=_ZdlPv,--wrap=_ZdlPvm,--wrap=memmove,--wrap=memcpy']
else:
    for name in ('Adafruit_GFX.cpp', 'Adafruit_GFX.h', 'glcdfont.c', 'gfxfont.h'):
        if not (GFX/name).is_file():
            raise SystemExit('SETUP REQUIRED: install Adafruit GFX or set ADAFRUIT_GFX_DIR; missing '+name)
        print(name, 'sha256='+hashlib.sha256((GFX/name).read_bytes()).hexdigest(), flush=True)
    flags += ['-I'+str(ROOT/'tests/transactions/stubs'), '-I'+str(ROOT/'tests/rasterized/stubs'),
              '-I'+str(GFX), '-I'+str(ROOT), str(ROOT/'tests/rasterized/test_terminal_scroll.cpp'),
              str(ROOT/'tests/host/allocation_hooks.cpp'), '-Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc',
              str(GFX/'Adafruit_GFX.cpp')]
binary = BUILD / ('host' if host else 'raster')
subprocess.run(flags+sources+['-o',str(binary)],check=True)
subprocess.run([str(binary)]+(['baseline'] if baseline else []),check=True,env=dict(os.environ,
    ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
