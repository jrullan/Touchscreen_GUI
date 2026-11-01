#!/usr/bin/env python3
"""Compile the real icon header; stubs model only Arduino's storage macro.

No board access. Host sections prove macro use, not MCU memory placement.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
NAMES = ('slider_on', 'slider_off', 'on_full', 'off_full',
         'lightbulb_on', 'lightbulb_off')
ARCHES = ('__AVR__', 'ESP8266', 'ESP32', 'ARDUINO_ARCH_STM32',
          'ARDUINO_ARCH_RP2040', 'ARDUINO_ARCH_SAMD')
STUB = '''#pragma once
#include <stdint.h>
#if defined(__AVR__)
#define PROGMEM __attribute__((section(".progmem.data")))
#elif defined(ESP8266)
#define PROGMEM __attribute__((section(".irom.text")))
#else
#define PROGMEM
#endif
'''


class IconHeaderTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='icon-contract-',
                                               dir=os.environ.get('TMPDIR'))
        self.addCleanup(self.tmp.cleanup)
        self.build = Path(self.tmp.name)
        (self.build / 'Arduino.h').write_text(STUB)
        self.source = self.build / 'icons.cpp'
        # External pointer table keeps every icon in the assembly, even at -Os.
        self.source.write_text('#include "icons.h"\nconst uint16_t* icons[] = {'
                               + ','.join(NAMES) + '};\n')

    def compile(self, arch, *options, compiler='g++'):
        return subprocess.run([compiler, '-std=c++11', '-D' + arch,
                               '-I' + str(self.build), '-I' + str(ROOT),
                               *options, str(self.source)], text=True,
                              capture_output=True)

    def test_syntax_supported_architectures(self):
        for arch in ARCHES:
            with self.subTest(architecture=arch):
                result = self.compile(arch, '-fsyntax-only')
                self.assertEqual(result.returncode, 0, result.stderr)

    def assert_flash_sections(self, compiler):
        asm = self.build / 'icons.s'
        result = self.compile('ESP8266', '-Os', '-S', '-o', str(asm),
                              compiler=compiler)
        self.assertEqual(result.returncode, 0, result.stderr)
        section = ''
        placed = set()
        assembly = asm.read_text()
        for line in assembly.splitlines():
            if '.section' in line:
                section = line
            for name in NAMES:
                if line.strip() == '_ZL' + str(len(name)) + name + ':':
                    self.assertIn('.irom', section, name + ': ' + section)
                    placed.add(name)
        self.assertEqual(placed, set(NAMES), 'Every icon must remain in flash')
        sizes = {}
        for name in NAMES:
            match = re.search(r'\.size\s+_ZL' + str(len(name)) + name
                              + r',\s*(\d+)', assembly)
            if match is None:
                self.fail(name + ': missing assembly size')
            sizes[name] = int(match.group(1))
            expected = 60 * 30 * 2 if name.startswith('slider_') else 50 * 50 * 2
            self.assertEqual(sizes[name], expected)
        print('\nFLASH SECTION', compiler, sizes, 'total_bytes=', sum(sizes.values()),
              flush=True)

    def test_esp8266_progmem_macro_used(self):
        self.assert_flash_sections('g++')

    def test_optional_xtensa_esp8266_sections(self):
        compiler = os.environ.get('XTENSA_CXX') or shutil.which('xtensa-lx106-elf-g++')
        if not compiler:
            candidates = sorted((Path.home() / '.arduino15/packages/esp8266/tools').glob(
                'xtensa-lx106-elf-gcc/*/bin/xtensa-lx106-elf-g++'))
            compiler = str(candidates[-1]) if candidates else None
        if not compiler:
            self.skipTest('xtensa-lx106-elf-g++ unavailable; parent runs target build')
        # Arduino.h normally exposes this toolchain header. Use its REAL PROGMEM
        # definition for target evidence, not our host approximation above.
        (self.build / 'Arduino.h').write_text(
            '#pragma once\n#include <stdint.h>\n#include <sys/pgmspace.h>\n')
        self.assert_flash_sections(compiler)


if __name__ == '__main__':
    unittest.main()
