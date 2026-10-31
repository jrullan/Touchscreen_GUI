#!/usr/bin/env python3
"""Compile real GUI sources with host hardware stubs and ASan/UBSan; no board access."""
from pathlib import Path
import os, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
BUILD=Path(os.environ.get('TMPDIR',str(ROOT/'.host-build')))/'touchscreen-gui-tests'
BUILD.mkdir(parents=True,exist_ok=True)
EXCLUDE={'Adafruit_FT6206.cpp','Adafruit_STMPE610.cpp','Seeedtouch.cpp','XPT2046_Touchscreen.cpp'}
sources=[str(p) for p in sorted(ROOT.glob('*.cpp')) if p.name not in EXCLUDE]
names=sys.argv[1:] or [p.stem.removeprefix('test_') for p in sorted((ROOT/'tests/host').glob('test_*.cpp'))]
for name in names:
 test=ROOT/'tests/host'/('test_'+name+'.cpp')
 binary=BUILD/name
 cmd=['g++','-std=c++17','-g','-O1','-fno-omit-frame-pointer','-fno-pie','-no-pie','-fsanitize=address,undefined','-Wno-write-strings','-Wno-conversion-null','-Wno-return-type','-DARDUINO=10819','-DESP8266','-I'+str(ROOT/'tests/host/stubs'),'-I'+str(ROOT),str(test),str(ROOT/'tests/host/allocation_hooks.cpp'),*([str(ROOT/'tests/host/lifecycle_odr.cpp')] if name=='lifecycle' else []),*sources,'-Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc','-o',str(binary)]
 print('BUILD',name,flush=True);subprocess.run(cmd,check=True)
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
 print('RUN',name,flush=True);subprocess.run([str(binary)],env=env,check=True)
 print('PASS',name,flush=True)
