"""Exercise the real allocation assertions with optimized, sanitized host builds.

Run with the desired g++ on PATH (including GCC 15+, which can eliminate unused
allocation pairs). Only optimization and the scratch output path change; source,
linker wrapping, sanitizers, and every allocation/byte oracle come from run.py.
"""
import os
from pathlib import Path
import runpy
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]


class AllocationCalibrationTest(unittest.TestCase):
    def test_optimized_allocation_calibration(self):
        scratch = Path(os.environ.get('TMPDIR') or Path.home() / '.cache' / 'touchscreen-gui-tests')
        scratch.mkdir(parents=True, exist_ok=True)
        real_run = subprocess.run
        for optimization in ('-O2', '-O3'):
            with self.subTest(optimization=optimization), tempfile.TemporaryDirectory(dir=scratch) as directory:
                compiled = []
                executed = []

                def run(command, *args, **kwargs):
                    if command[0] == 'g++':
                        self.assertIn('-O1', command)
                        self.assertIn('-fsanitize=address,undefined', command)
                        command = [optimization if item == '-O1' else item for item in command]
                        compiled.append(command)
                    else:
                        executed.append(command)
                    # Execute, do not mock, both the compiler and the resulting test.
                    return real_run(command, *args, **kwargs)

                with patch.dict(os.environ, TMPDIR=directory), patch.object(
                    sys, 'argv', [str(ROOT / 'tests/host/run.py'), 'optional_allocations']
                ), patch('subprocess.run', side_effect=run):
                    runpy.run_path(sys.argv[0], run_name='__main__')
                self.assertEqual(len(compiled), 1)
                self.assertEqual(len(executed), 1)


if __name__ == '__main__':
    unittest.main()
