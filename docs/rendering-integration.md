# Rendering integration: measured results and remaining review

This work intentionally selects rendering efficiency (Phase 3 in the original plan) ahead of low-allocation storage. It is isolated on `perf/render-batching`, based on safety PR #3 at `ad5abc5`.

## Implemented behavior

- Trend/MultiTrend deferred sample mutations do not touch the display.
- A commit erases recorded old projected geometry before restoring grid/threshold layers, new lines/markers, and border. Ordinary changed frames do not wipe the whole widget.
- Unchanged committed charts generate zero drawing operations. `invalidate()` schedules a repaint; `show()` and `forceUpdate()` repair external damage.
- Display `setText(text,true)` renders only when effective retained text or style/geometry changed. Display `update()`, `show()`, and `forceUpdate()` remain unconditional repaint paths.
- Caches have checked allocation and documented safe fallback. Persistent chart cache OOM falls back to full redraw rather than promising zero traffic.

## Actual measurements

Same unchanged `Phase01_Baseline` sketch bytes, ESP8266 D1 Mini, core 3.1.2, three series of 32 samples, first 21 updates, no WiFi, default display SPI rate:

| Measurement | Safety version | Rendering version |
| --- | ---: | ---: |
| Median update | 21,536 us | 14,826 us |
| Free heap after widgets | 50,504 bytes | 49,688 bytes |
| Observed minimum reported free continuation stack | 3,312 bytes | 3,024 bytes |

Median time decreased 31.16% for this workload, at 816 extra heap bytes for this sketch's widgets/caches/allocator overhead. This is not a dashboard frame-time, universal speedup, endurance or SPI-bandwidth claim. All sample times and sketch SHA256 are in `rendering-device-benchmark.json`.

Host operation-count workloads: chart 20 changed frames 65,722 -> 22,160; 20 unchanged updates 26,540 -> 0. Display 100 identical values 200 -> 0; 100 changed values 200 -> 200. Host operation counts are not SPI transaction counts.

## Verification performed

- Eleven actual-source ASan/UBSan/leak suites passed; strict return/const/sentinel checks passed.
- Seven clean exact-library ESP8266 builds passed.
- Existing full-widget demo also compiled; legacy example string-literal warnings remain. The optimized demo has not been flashed/physically accepted yet.
- Optimized dashboard uploaded after a clean rebuild, flash hash verified; serial showed WiFi connection, initial draw completion, four HTTP 200 polls in a 25-second capture and no observed exception/watchdog markers.
- Physical dashboard appearance remains for user verification. Final independent chart and Display re-reviews passed with no confirmed blocking regressions; reviewers independently reran chart, Display and real-GFX raster suites. Backend font/wrap changes require explicit Display update(), forceUpdate(), or show(); backend text configuration is not part of identical-value suppression.

## Clean-build regression

The first optimized dashboard upload from a non-clean Arduino build reset with unhandled OOM while creating charts. The call site attempted the old 168-byte MultiTrend allocation despite the new layout being 328 bytes. The known-good dashboard was immediately restored. A `--clean` rebuild changed the produced firmware and the optimized dashboard subsequently booted and polled successfully. The build checker now always requests `--clean`; `tests/tools/test_check_arduino.py` reproduced missing clean arguments before the fix and passed after it. That Python test mocks subprocess solely to assert command construction, not to synthesize device/build results. CI includes the command regression.

Device logs/build artifacts are preserved in active-profile scratch under `gui-rendering-bench` and `gui-rendering-builds`. Firmware can include compiled WiFi credentials; do not publish binaries or copy secret.h into this repository. Installed/vendored libraries remain unchanged; device currently runs the clean optimized dashboard.
