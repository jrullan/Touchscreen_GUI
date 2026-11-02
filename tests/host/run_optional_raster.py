#!/usr/bin/env python3
"""Exercise the existing real-GFX oracles with only constructor policy changed.

Keep raster fixtures/runners untouched. Redirect their g++ test input to a scratch
copy; all library sources, sanitizer flags, case selection and oracles are real.
"""
from pathlib import Path
import os
import runpy
import subprocess
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
BUILD = Path(os.environ.get('TMPDIR') or Path.home() / '.cache' / 'touchscreen-gui-tests') / 'gui-optional-text-raster'
BUILD.mkdir(parents=True, exist_ok=True)
real_run = subprocess.run
for suite, replacements in (
    ('keyboard', {'Keyboard k;': 'Keyboard k(Button::TextStorage::None); assert(k.contents==nullptr);'}),
    ('icon_runs', {
        'IconButton icon(w,h,bitmap);': 'IconButton icon(w,h,bitmap,nullptr,(char*)"",Button::TextStorage::None); assert(icon.contents==nullptr);',
        'IconButton hidden(1,1,&pixel);': 'IconButton hidden(1,1,&pixel,nullptr,(char*)"",Button::TextStorage::None); assert(hidden.contents==nullptr);',
        'IconButton detached(1,1,&pixel);': 'IconButton detached(1,1,&pixel,nullptr,(char*)"",Button::TextStorage::None); assert(detached.contents==nullptr);',
    }),
):
    source = ROOT / 'tests/rasterized' / ('test_' + suite + '.cpp')
    text = source.read_text()
    for old, new in replacements.items():
        assert old in text, 'fixture constructor changed: ' + old
        text = text.replace(old, new)
    alternate = BUILD / source.name
    alternate.write_text('#include <cassert>\n' + text)

    def redirect(command, *args, **kwargs):
        if isinstance(command, list) and command[0] == 'g++':
            assert str(source) in command
            command = [str(alternate) if item == str(source) else item for item in command]
        return real_run(command, *args, **kwargs)

    print('OPT-IN None: real existing ' + suite + ' raster suite', flush=True)
    saved_argv = sys.argv
    sys.argv = [str(ROOT / 'tests/rasterized' / ('run_' + suite + '.py'))]
    try:
        with patch('subprocess.run', redirect):
            try:
                runpy.run_path(sys.argv[0], run_name='__main__')
            except SystemExit as error:
                if error.code not in (None, 0):
                    raise
    finally:
        sys.argv = saved_argv
