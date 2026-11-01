"""Build-command regression only; subprocess is mocked, no board claims."""
import contextlib
import io
import os
from pathlib import Path
import runpy
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

class CleanBuildTest(unittest.TestCase):
    def test_exact_library_builds_reject_stale_sketch_objects(self):
        root = Path(__file__).resolve().parents[2]
        calls = []
        def record(args, **kwargs):
            calls.append(args)
            return subprocess.CompletedProcess(args, 0, stdout='test fixture\n')
        with tempfile.TemporaryDirectory(dir=os.environ.get('TMPDIR')) as out:
            with patch.object(sys, 'argv', ['check_arduino.py', '--output', out]), patch('subprocess.run', side_effect=record), contextlib.redirect_stdout(io.StringIO()):
                runpy.run_path(str(root / 'tools/check_arduino.py'), run_name='__main__')
        builds = [args for args in calls if 'compile' in args]
        self.assertTrue(builds)
        self.assertTrue(any(Path(args[-1]).name == 'touch_mapper' for args in builds),
                        'Opt-in touch mapper must compile in the compatibility matrix')
        for fixture in ('touch_ft6206', 'touch_stmpe610', 'keyboard'):
            self.assertTrue(any(Path(args[-1]).name == fixture for args in builds),
                            fixture + ' must compile against the real controller API')
        for args in builds:
            self.assertIn('--clean', args)
            self.assertIn('--library', args)

if __name__ == '__main__':
    unittest.main()
