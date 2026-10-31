#!/usr/bin/env python3
"""Isolated real-GFX transaction dispatch tests; no shared stubs modified."""
from pathlib import Path
import os
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[2]
GFX=Path(os.environ.get("ADAFRUIT_GFX_DIR",str(Path.home()/"Arduino/libraries/Adafruit_GFX_Library")))
if any(not (GFX/n).is_file() for n in ("Adafruit_GFX.cpp","Adafruit_GFX.h","glcdfont.c")):
    sys.exit("SETUP REQUIRED: install Adafruit GFX or set ADAFRUIT_GFX_DIR")
BUILD=Path(os.environ.get("TMPDIR",str(ROOT/".host-build")))/"touchscreen-gui-transactions"
BUILD.mkdir(parents=True,exist_ok=True)
cmd=["g++","-std=c++17","-g","-O1","-fno-omit-frame-pointer","-fno-pie","-no-pie",
     "-fsanitize=address,undefined","-DARDUINO=10819","-DESP8266",
     "-I"+str(ROOT/"tests/transactions/stubs"),"-I"+str(ROOT/"tests/rasterized/stubs"),
     "-I"+str(GFX),"-I"+str(ROOT),str(ROOT/"tests/transactions/test_transactions.cpp"),
     str(ROOT/"GUI_TFT.cpp"),str(GFX/"Adafruit_GFX.cpp"),"-o",str(BUILD/"transactions")]
print("GFX SOURCE:",GFX,flush=True)
subprocess.run(cmd,check=True)
subprocess.run([str(BUILD/"transactions")],check=True,env=dict(os.environ,
    ASAN_OPTIONS="detect_leaks=1:halt_on_error=1",UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1"))
