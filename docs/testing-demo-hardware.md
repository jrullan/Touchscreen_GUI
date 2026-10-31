# Existing demo: compatibility and physical acceptance

## Verified execution

- Checkpoint `fb9912b` contains the reviewed Phase 0/1 safety work.
- Built a rollback dashboard image using the unchanged installed library, explicitly selected with `--library /home/jrullan/Arduino/libraries/Touchscreen_GUI`.
- Compiled the existing `examples/Touchscreen_GUI_Demo` against this refactor tree and `jrullan/neotimer` (reported version 1.1.6), selected explicitly with `--library`.
- The original demo exposed an ambiguous `setCV(map(...))` call: Arduino map returns long. Added an exact signed-long overload delegating to the existing int64 clamping helper, rather than casting the demo calls.
- RED: `test_map_compat.cpp` failed to compile with ambiguous long overload calls. GREEN: the same suite passed for map results, LONG_MIN/MAX, negative int and UINT_MAX. An independent reviewer reran it under ASan/UBSan and found no security or logic issues in the scoped fix.
- Strict compiler checks and all nine host suites passed after the fix.
- Demo build: static RAM 48,360 bytes, executable IRAM 27,847 bytes plus 32,768-byte instruction cache reservation, flash code 273,072 bytes. The legacy example still emits string-literal-to-char-pointer warnings; do not describe this demo build as warning-free.
- Uploaded demo to /dev/ttyUSB0; esptool identified ESP8266EX MAC 18:fe:34:d2:ed:20 and reported flash hash verified, then reset the board.
- A 12-second serial capture contained no recognized exception/watchdog markers. The demo has no startup diagnostic banner; this capture does not prove successful GUI initialization or heap stability.

## Pending user/device acceptance

The demo remains loaded for physical inspection. Verify the main screen and Terminal, Dial screen via the published demo PIN, slider response, icon-button toggles and repeated screen switching. These have not been independently observed by the assistant. Dynamic heap stability, calibrated touch accuracy and network recovery remain unverified. Do not restore the dashboard until the user finishes inspecting the demo.

The rollback dashboard image and demo artifacts are in the active profile scratch directory under `gui-phase01-hardware/`; they contain firmware and must not be published indiscriminately. Installed and vendored libraries have not been replaced. No follow-up PR is opened by this checkpoint.
