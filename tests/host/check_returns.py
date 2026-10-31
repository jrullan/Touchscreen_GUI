#!/usr/bin/env python3
"""Compile strict-return checks without linking hardware implementations."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2]
args=['g++','-std=c++17','-Werror=return-type','-DARDUINO=10819','-DESP8266','-I'+str(r/'tests/host/stubs'),'-I'+str(r),'-fsyntax-only']
subprocess.run([*args,str(r/'GUI_TFT.cpp')],check=True)
subprocess.run([*args,'-x','c++','-'],input='#include "Seeedtouch.h"\n',text=True,check=True)
subprocess.run([*args,'-Werror=write-strings','-Werror=conversion-null',*[str(r/f) for f in ('OptionButton.cpp','Popup.cpp','Numkey.cpp','Terminal.cpp')]],check=True)
print('PASS strict return and const/sentinel checks')
