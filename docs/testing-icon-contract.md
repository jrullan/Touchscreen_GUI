# Icon assets and IconButton contract

Scope: `icons.h`, `IconButton.h`, `IconButton.cpp`; no changes to generic
Button/Canvas touch processing, no automatic redraw, no Designer changes.

## Run

```sh
python3 tests/tools/test_icons.py -v
python3 tests/host/run.py icon_contract
python3 tests/host/run.py
```

The header tests invoke **g++ on the actual icons.h**, not a textual imitation.
Minimal Arduino.h stubs supply only integer types and the target's storage macro:
AVR uses a program-memory section, ESP8266 an irom section, and memory-mapped
ESP32/STM32/RP2040/SAMD targets an empty PROGMEM macro. Each of these architecture
macros is compiled independently. This is header portability evidence, not a full
build of the library for each MCU.

The optional Xtensa test discovers `xtensa-lx106-elf-g++` from PATH or the Arduino
ESP8266 package, or accepts `XTENSA_CXX=/absolute/path/to/xtensa-lx106-elf-g++`.
It uses the **real toolchain `<sys/pgmspace.h>` PROGMEM definition**, normally
exposed by Arduino.h, through a minimal Arduino.h boundary. It compiles assembly
and requires all six asset labels to reside in `.irom...` sections, with the
expected byte sizes. An unavailable toolchain produces an explicit skip. Parent
integration remains responsible for a full target build; these tests do not flash.

The host test links the actual production IconButton, Button, Widget, Canvas and
other core sources through the existing ASan/UBSan/leak-check runner. Only
Arduino and display/hardware boundaries are stubbed. Pixel-buffer assertions
exercise the actual bitmap drawing implementation, which retains `pgm_read_word`.
Host reads alone cannot validate physical flash bus accesses.

## Intended behavior

- AVR **and ESP8266** assets use Arduino's PROGMEM. ESP8266 ordinary `const`
  previously placed their bytes in `.rodata` (DRAM in the usual target layout),
  rather than flash irom sections. Other targets receive valid const array
  declarations through the fallback branch.
- IconButton's constructor uses its own **500 ms** default, independent of
  Canvas's unchanged global `DEBOUNCE == 0`. No per-object field is added.
- `setDebounce(0)` remains valid, and explicit nonzero overrides remain valid.
  Inherited Button comparison is still strictly `lastMillis + debounceTime <
  millis()`: equality does not trigger, even with a zero interval.
- This is time throttling, **not one event per physical press**. A continuously
  held in-bounds point can toggle again after the interval; no edge/release
  tracking or wraparound redesign is introduced.
- No callback: inherited checkTouch returns false without changing `touched` or
  `lastMillis` and without drawing. A callback permits toggling before invocation.
- A callback which does not call update changes state but does not redraw.
  `button.update()` explicitly renders that state. A callback receiving `Button*`
  can call virtual `update()` to render immediately. This remains caller policy.
- `imagePressed == nullptr` uses the unpressed image even when touched; transparent
  pixels retain existing panel pixels unless `useTransparency` is false. Hidden
  updates do not paint.

These callback/rendering cases characterize existing behavior (audit finding 3),
not fixes requiring a new rendering policy.

## Recorded RED -> GREEN evidence

Base: `cd9294b9c1babe268cbef07a58adeadc64c8685c`, branch
`fix/designer-widget-findings`, shared widget-fixes worktree.

1. `python3 tests/tools/test_icons.py IconHeaderTests.test_syntax_supported_architectures -v`
   **RED:** three subtest failures, STM32/RP2040/SAMD; compiler reported
   `expected unqualified-id before numeric constant` because declarations were
   absent. Changed the six ESP-only alternate branches to fallback `#else`.
   **GREEN:** same command passed all six architecture compilations.
2. Ran the ESP8266 host-section and optional Xtensa-section tests before the
   storage fix. **RED:** both found `lightbulb_off` in `.rodata` instead of `.irom`.
   Added ESP8266 to the existing AVR PROGMEM branch in all six declarations.
   **GREEN:** `python3 tests/tools/test_icons.py -v` passed all three test methods,
   including actual Xtensa assembly. The final optional test also passed using
   real toolchain pgmspace.h rather than the host macro approximation.
3. `python3 tests/host/run.py icon_contract` initially exposed a test setup error
   (Point requires three constructor arguments); corrected the fixture and reran
   before editing production. **RED:** runtime assertion
   `calls == 0 && !button.touched` failed at 1001 ms, one millisecond after
   construction, proving the inherited global zero default leaked into IconButton.
   Replaced the constructor's DEBOUNCE reference with its own static 500 ms
   constant and removed the ineffective fallback macro definition.
   **GREEN:** same target passed, then passed again with zero/custom interval and
   callback/rendering characterization cases added.
4. `python3 tests/host/run.py`: **all 23 discovered executables passed**, including
   concurrent agents' buttongrid_highlight, legacy_input_contracts and xaxis_spacing
   tests. No sanitizer diagnostics. `git diff --check` passed.

## Memory evidence and limits

Xtensa compiler observed:
`~/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/3.1.0-gcc10.3-e5f9fec/bin/xtensa-lx106-elf-g++`.

Assembly sizes for each retained asset:

| Asset | Bytes |
| --- | ---: |
| slider_on | 3600 |
| slider_off | 3600 |
| on_full | 5000 |
| off_full | 5000 |
| lightbulb_on | 5000 |
| lightbulb_off | 5000 |
| **Total** | **27200** |

All six were in irom sections with the real Xtensa PROGMEM macro. Independent
comparison against `git show cd9294b:icons.h`, excluding comments, confirmed all
13,600 RGB565 numeric words unchanged and in the same order. This moves asset
storage; it does not alter image payloads or prove a whole-sketch DRAM delta after
linker garbage collection. Only assets retained by a sketch contribute.

Compiling host sizeof probes against both baseline and current IconButton headers
returned **128 bytes in both cases**; the production host test also reports
`sizeof(IconButton)=128 sizeof(Button)=104`. These are host ABI measurements, not
MCU ABI sizes. A static integral constant adds no per-instance storage.

Asset definitions and their linkage organization are deliberately retained;
include these large asset definitions in one translation unit rather than
assuming repeated header inclusion is free. No broad extern/inline rewrite,
physical touch test, full board compilation, flash, commit or push was performed
by this task.
