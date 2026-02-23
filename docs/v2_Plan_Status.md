# Touchscreen_GUI v2 — Plan Status

Last updated: February 2026

## Overview

The v2 plan addresses two systemic bottlenecks: (1) the rendering pipeline redraws far more pixels than necessary, and (2) the touch system has high latency and limited flexibility. Work is organized into 5 incremental phases. Each phase leaves the library in a working state.

Full plan details: see `.claude/plans/imperative-growing-hamster.md`

---

## Phase 1: Quick Wins — PARTIALLY COMPLETE

Constant tuning, dead code removal, dirty flag.

| Item | Status | Notes |
|------|--------|-------|
| `SCAN_SAMPLING_TIME` 100→20 | Done | |
| `TOUCH_SAMPLING_TIME` 25→0 | Done | |
| Remove dead touch buffers from Canvas.h | Done | |
| Remove platform analog pin `#ifdef` blocks | Done | |
| `Widget._dirty` flag | Done | |
| `Button.debounceTime` 500→150 | Done | |
| `Indicator.setCV()` dirty flag | Done | |
| `Display.update()` dirty check | Done | |
| `Gauge.update()` dirty check | Done | |
| `Popup.update()` implementation | Not started | |
| Delete Canvas_SEEEDTOUCH.h | Done | |
| Delete Canvas_FT6206.h | Done | |
| Delete Canvas_STMPE610.h | Done | |
| Delete Seeedtouch.h/cpp | Done | |
| Delete Adafruit_FT6206.h/cpp | Done | |
| Delete Adafruit_STMPE610.h/cpp | Done | |

---

## Phase 2: Rendering Optimizations — PARTIALLY COMPLETE

SPI batching, delta draws, circular buffer, terminal scroll.

| Item | Status | Notes |
|------|--------|-------|
| 2.1 startWrite/endWrite batching (Dial) | Done | Dial.cpp wraps in startWrite/endWrite |
| 2.1 startWrite/endWrite batching (other widgets) | Not started | Gauge, Slider, Trend, Terminal, Display, Button |
| 2.2 Delta-only rendering (Gauge, Slider) | Not started | Only redraw the changed strip |
| 2.3 Circular buffer for Trend | Not started | O(1) addValue instead of O(n) shift |
| 2.4 Terminal scroll optimization | Not started | memcpy, dirty lines, clear only vacated line |

### Expected Impact

Delta rendering (2.2) reduces pixel count per frame — multiplicative with the SPI fast path. If Gauge/Slider only redraw the changed strip instead of the full bar, the number of pixels drops proportionally to the value change step size.

---

## Phase 2.5: SPI Fast Path — COMPLETE

Custom `writeColorFast` bypassing Adafruit's `delay(1)` bottleneck on ESP8266/ESP32.

| Item | Status | Notes |
|------|--------|-------|
| `writeColorFast` using `SPI.writeBytes()` | Done | 64-byte FIFO batches (32 pixels) |
| 6 virtual method overrides in Guitft | Done | writeFillRect, writeFastHLine, writeFastVLine + transaction-wrapped variants |
| Platform guard (`#if ESP8266 \|\| ESP32`) | Done | STM32 path unaffected |
| Benchmark verification | Done | 27.5x dial speedup (64,400→2,342 us) |

See [SPI_Fast_Path_Benchmark_Results.md](SPI_Fast_Path_Benchmark_Results.md) for detailed measurements.

---

## Phase 3: Touch Architecture — COMPLETE

New touch driver interface, calibration struct, coordinate mapper with state machine.

| Item | Status | Notes |
|------|--------|-------|
| `TouchDriver.h` abstract interface | Done | `begin()`, `touched()`, `readRaw()`, `touchCount()`, `needsFiltering()` |
| `TouchCalibration.h` | Done | Calibration struct + `TouchCal::XPT2046_DEFAULT` preset |
| `TouchMapper.h/cpp` | Done | Orientation mapping, median-of-3 filtering, state machine |
| `TouchDriver_XPT2046.h` | Done | Thin adapter for XPT2046_Touchscreen |

---

## Phase 4: Advanced Rendering — PARTIALLY COMPLETE

Hardware scroll and region-scoped redraw.

| Item | Status | Notes |
|------|--------|-------|
| 4.1 `setScrollArea()` / `setScrollStart()` in GUI_TFT | Done | ILI9341 commands 0x33/0x37 |
| 4.1 `useHardwareScroll` in Terminal | Not started | Portrait-only, landscape falls back to software |
| 4.2 `Canvas::redrawRegion()` | Not started | Region-scoped widget repaint |

---

## Phase 5: Integration — COMPLETE

Wire new touch system into Canvas, unified Canvas class.

| Item | Status | Notes |
|------|--------|-------|
| 5.1 `Canvas_Touch` class | Done | Works with any TouchDriver via TouchMapper |
| 5.2 All 4 orientations | Done | TFT_PORTRAIT, LANDSCAPE, PORTRAIT2, LANDSCAPE2 |
| 5.3 Deprecate Canvas_XPT2046 | Done | Kept functional, new API recommended |
| 5.4 Update demo example | Done | Uses Canvas_Touch + TouchDriver_XPT2046 |

---

## Remaining Work — Priority Order

### High Impact (rendering performance)

1. **Phase 2.2: Delta-only rendering for Gauge and Slider** — Only redraw the strip between previous and current value. Multiplicative with SPI fast path since it reduces pixel count.

2. **Phase 2.1: startWrite/endWrite batching for remaining widgets** — Gauge, Slider, Trend, Terminal, Display, Button. Eliminates redundant SPI transaction overhead.

3. **Phase 2.4: Terminal scroll optimization** — `memcpy` for line shifting, dirty line tracking, clear only the vacated line. Significant for Terminal-heavy applications.

### Medium Impact

4. **Phase 2.3: Circular buffer for Trend** — O(1) addValue. Matters when Trend has many data points.

5. **Phase 4.1: Hardware scroll for Terminal** — Uses ILI9341 hardware scroll command for zero-pixel-copy vertical scrolling in portrait mode.

6. **Phase 4.2: Region-scoped redraw** — `Canvas::redrawRegion()` for Popup/Numkey hide. Avoids full-screen repaint.

### Low Impact

7. **Phase 1: Popup.update()** — Minor; Popup is rarely used in a tight loop.
