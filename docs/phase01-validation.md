# Phase 0/1: running the safety gates

Work is based on upstream merge `46b03f5`. This branch is a follow-up to merged PR #2; it does not update that PR or the installed Arduino library automatically.

## Host regression suite

Requires Python 3.9+ and g++ with AddressSanitizer/UndefinedBehaviorSanitizer.

```sh
python3 tests/host/check_returns.py
python3 tests/host/run.py
```

The suite compiles real widget/core sources. Arduino/display stubs replace hardware, not widget behavior. It enables leak detection and exercises C allocation failures; `test_new_oom.cpp` also injects nothrow C++ object-allocation failure for Display, Button and Canvas. Lifecycle compilation includes a second translation unit. Build products go under `$TMPDIR/touchscreen-gui-tests` (or local `.host-build` when TMPDIR is unset).

Text is recorded as text/content/bounds, not rasterized with the real font. Some primitive operations have a clipped pixel buffer; the current triangle stubs are no-ops, so Dial needle geometry/erasure is not visually verified. One-sample chart tests establish safe storage/access, not a visible plotted sample. Passing host tests is not proof of physical rendering, driver I/O behavior, ABI-equivalent object sizes, or performance.

## Exact-library ESP8266 build matrix

```sh
python3 tools/check_arduino.py \
  --cli /path/to/arduino-cli \
  --fqbn esp8266:esp8266:d1_mini \
  --output /path/outside/repository \
  --dashboard /path/to/tft-panelboard/firmware/tft_panelboard
```

`--dashboard` is optional; its gitignored secret.h must already exist. Never copy credentials into this library. The tool explicitly selects this library tree using `--library`; it does not silently validate a different installed copy. Individual logs and firmware artifacts are saved in the requested output directory. Compilation never uploads.

Workloads: display (including a second translation unit), input, terminal, Trend/MultiTrend, optional on-board instrumentation, and dashboard. Library dependencies are declared in `library.properties`. Neotimer is needed only by the older demonstration sketches that include it, not these safety workloads.

Only ESP8266 core 3.1.2 is installed in this environment. AVR, ESP32, SAM and STM32 results must not be inferred from a successful ESP8266 build. The architecture wildcard in Arduino metadata preserves library discoverability, not a cross-target certification.

## Optional physical baseline

`examples/Phase01_Baseline` reports object sizes, free heap/largest block/fragmentation, free continuation stack on ESP8266, and chart-update duration. It has been compiled; physical values are not available until a separately authorized upload/run. Do not present compile-time RAM totals as dynamic heap or timing measurements.

## Lifetime and compatibility contracts

- Canvas/Screen registries borrow widgets; they do not delete them. Remove registrations before destroying a widget; detach a surviving Screen before destroying its Canvas. Do not destroy active registered objects in event callbacks. Automatic detach/reentrant dispatch is not introduced here.
- Canvas owns its created display and touch drivers. Do not replace public driver pointers with borrowed storage; custom driver owners must coordinate destruction explicitly.
- Existing label/message setters now retain owned copies. Getter/public buffer pointers remain borrowed views; do not free or rebind them. Allocation metadata is not a direct resize API.
- Owning Widget/Text/Canvas/Screen/StackArray instances cannot be shallow-copied. Construct them in place and register pointers. This intentionally rejects unsafe old copies; old C++11 copy-initialization idioms may require direct initialization. No move API is introduced in this safety pass.
- Widget hide now disables visibility; explicit show restores visibility. Full visibility/screen activation redesign remains Phase 5.
- Grid IDs remain one-based. At most 255 grid cells; labels are owned eight-character copies. Numkey remains a six-character entry field and truncates oversized decimal strings safely.
- Histories retain newest-first, series-major public storage and linear-shift insertion. Capacity changes require reinitialization; direct writable pointers/fields are not a license to overrun allocated storage.
- Constructor-started hardware initialization is retained. XPT2046/FT6206/STMPE610 publish `ts` only after successful begin and retain failed drivers privately for retry. STMPE610 attempts begin once per `init()` instead of looping forever; a failed attempt requires an explicit later `init()` call. Underlying hardware calls may still block.
- Unsigned-int scale/current-value APIs preserve values through `UINT_MAX`; signed negative current values clamp to the scale minimum. Added overloads can require explicit casts for wider integer callers or untyped member-function addresses. Histories remain byte-valued.
- Handlerless Buttons/Sliders can still swallow input outside their bounds. Callback propagation and Slider debounce rollover remain deferred; see review regression notes.

## Completed software verification

- Strict return/const/sentinel compiler checks: passed.
- Eight ASan/UBSan/leak-checking suites: charts, core, lifecycle (two translation units), new_oom, review_ranges, review_startup, text, visibility — passed.
- Seven exact-library ESP8266 builds: display, input, terminal, trend, sizes, instrumentation, dashboard — passed.
- Final independent review of range, geometry, visibility and touch-startup fixes: passed with no confirmed blocking regressions; reviewer independently reran both review suites and signed-API compile checks.
- Whitespace diff checks: passed.
- Target object sizes and compiler memory totals are recorded in `phase01-build-results.json`; dynamic allocations, runtime timing and peak stack are not inferred from those totals.

The individual testing-*.md documents record red/green observations for each implementation stream. Physical display/touch validation, real-board heap stability and hardware timing remain outstanding. This software verification does not claim a device upload or certification of other board architectures.
