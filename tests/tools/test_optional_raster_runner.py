"""Runner environment regressions; nested raster execution is mocked, not compiled."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
PROBE = r'''
from pathlib import Path
import os
import runpy
import sys
from unittest.mock import patch

runner = Path(sys.argv[1])
expected_build = Path(sys.argv[2]) / 'gui-optional-text-raster'
real_run_path = runpy.run_path
seen = []

def check_suite(path, run_name):
    suite = Path(path).stem.removeprefix('run_')
    assert suite in ('keyboard', 'icon_runs'), path
    assert run_name == '__main__'
    alternate = expected_build / ('test_' + suite + '.cpp')
    assert alternate.is_file(), alternate
    assert 'Button::TextStorage::None' in alternate.read_text()
    seen.append(suite)

with patch('runpy.run_path', side_effect=check_suite):
    real_run_path(str(runner), run_name='__main__')
assert seen == ['keyboard', 'icon_runs'], seen
'''


class OptionalRasterRunnerTest(unittest.TestCase):
    def test_unset_tmpdir_uses_user_cache(self):
        scratch = Path(os.environ.get('TMPDIR') or Path.home() / '.cache' / 'touchscreen-gui-tests')
        scratch.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=scratch) as directory:
            home = Path(directory) / 'home'
            env = dict(os.environ, HOME=str(home))
            env.pop('TMPDIR', None)
            expected = home / '.cache' / 'touchscreen-gui-tests'
            result = subprocess.run(
                [sys.executable, '-c', PROBE,
                 str(ROOT / 'tests/host/run_optional_raster.py'), str(expected)],
                env=env, capture_output=True, text=True,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.splitlines(), [
                'OPT-IN None: real existing keyboard raster suite',
                'OPT-IN None: real existing icon_runs raster suite',
            ])


if __name__ == '__main__':
    unittest.main()
