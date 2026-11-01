#!/usr/bin/env python3
"""Compile the exact working library; logs/artifacts never enter the library tree."""
import argparse, os, subprocess
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument('--cli',default='arduino-cli');p.add_argument('--fqbn',default='esp8266:esp8266:d1_mini');p.add_argument('--output',required=True);p.add_argument('--dashboard',type=Path);a=p.parse_args()
r=Path(__file__).resolve().parents[1];out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
subprocess.run([a.cli,'version'],check=True)
subprocess.run([a.cli,'core','list'],check=True)
subprocess.run([a.cli,'lib','list'],check=True)
sketches=[(n,r/'tests/arduino'/n) for n in ('display','input','terminal','trend','sizes','touch_mapper','touch_ft6206','touch_stmpe610','keyboard')]
sketches.append(('instrumentation',r/'examples/Phase01_Baseline'))
if a.dashboard:sketches.append(('dashboard',a.dashboard.resolve()))
for name,sketch in sketches:
 cmd=[a.cli,'compile','--clean','--fqbn',a.fqbn,'--library',str(r),'--output-dir',str(out/name),str(sketch)]
 print('BUILD',name,flush=True)
 result=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 (out/(name+'.log')).write_text(result.stdout)
 print(result.stdout,flush=True)
 if result.returncode:raise SystemExit(result.returncode)
print('PASS all exact-library Arduino builds')
