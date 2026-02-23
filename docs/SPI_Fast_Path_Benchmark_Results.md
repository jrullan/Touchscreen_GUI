# SPI Fast Path — Benchmark Results

## Date: February 2026

## Hardware

| Parameter | Value |
|-----------|-------|
| MCU | ESP8266EX (WEMOS D1 Mini) @ 80 MHz |
| SPI clock | HSPI @ 40 MHz |
| Display | ILI9341 (Lolin TFT-2.4), 240x320, RGB565 |
| SPI FIFO | 64 bytes |
| Board core | esp8266:esp8266@3.1.2 |

## What Changed

Overrode 6 virtual rendering methods in `Guitft` (our `Adafruit_ILI9341` subclass) to bypass `Adafruit_SPITFT::writeColor()`, which contains a `delay(1)` on every call on ESP8266. The overrides use `SPI.writeBytes()` with a pre-filled 64-byte buffer (32 pixels per FIFO fill) for bulk writes at full SPI clock speed.

See [ESP8266_Dial_Theoretical_Performance.md](ESP8266_Dial_Theoretical_Performance.md) for the theoretical analysis that motivated this change.

### Files Modified

- `GUI_TFT.h` — 6 virtual method override declarations + private `writeColorFast`, guarded by `#if defined(ESP8266) || defined(ESP32)`
- `GUI_TFT.cpp` — Implementation of `writeColorFast` using `SPI.writeBytes()` + 6 overrides with Adafruit-identical clipping logic

### Methods Overridden

| Method | Context | Purpose |
|--------|---------|---------|
| `writeFillRect` | Transaction-less | Called by `fillTriangle`, `fillCircle`, `drawChar` |
| `writeFastHLine` | Transaction-less | Called by `fillTriangle`, scanline fills |
| `writeFastVLine` | Transaction-less | Called by `fillCircle`, `fillCircleHelper` |
| `fillRect` | Transaction-wrapped | Standalone public API |
| `drawFastHLine` | Transaction-wrapped | Standalone public API |
| `drawFastVLine` | Transaction-wrapped | Standalone public API |

No Adafruit library files were modified. STM32 path is unaffected.

## Benchmark Methodology

The demo auto-starts on the Dial & Slider screen with the slider animation running. The slider sweeps 0-100 in steps of 2 every 10ms. The dial maps the slider value to range 60-90. Per-widget draw times are measured with `micros()`. Results are reported via Serial after the benchmark window.

## Results

### writeBytes Implementation (Final — 10s benchmark)

```
===== SPI FAST PATH BENCHMARK (10s) =====
Total frames:  10451
Draw frames:   822
Avg loop:      583 us
Avg slider:    114 us
Avg dial:      2342 us
Total slider:  94192 us
Total dial:    1925340 us
========================================
```

Note: Avg loop includes ~first second of boot/screen init. The 5s run showed 80 us avg loop (pure animation phase only).

### writeBytes Implementation (5s benchmark, animation-only)

```
===== SPI FAST PATH BENCHMARK (5s) =====
Total frames:  37989
Draw frames:   411
Avg loop:      80 us
Avg slider:    114 us
Avg dial:      2341 us
Total slider:  46945 us
Total dial:    962382 us
========================================
```

### write16 Implementation (earlier attempt — 5s benchmark)

```
===== SPI FAST PATH BENCHMARK (5s) =====
Total frames:  17165
Draw frames:   362
Avg loop:      191 us
Avg slider:    333 us
Avg dial:      3802 us
Total slider:  120600 us
Total dial:    1376354 us
========================================
```

## Performance Comparison

### Dial Rendering (Avg per draw frame)

| Version | Avg Dial (us) | Avg Slider (us) | Avg Loop (us) | Speedup vs Previous |
|---------|--------------|-----------------|---------------|-------------------|
| Pre-v2 (Adafruit writeColor) | 64,400 | — | — | baseline |
| v2 Phase 1-2 (dirty flags, SPI batching) | 44,100 | — | 50,800 | 1.5x |
| SPI Fast Path — write16 | 3,802 | 333 | 191 | 11.6x |
| **SPI Fast Path — writeBytes** | **2,342** | **114** | **80** | **1.6x** |

### Cumulative Improvement

| Metric | Pre-v2 | Final (writeBytes) | Total Speedup |
|--------|--------|-------------------|---------------|
| Avg dial | 64,400 us | **2,342 us** | **27.5x** |
| Avg loop | ~50,800 us | **80 us** | **635x** |
| Per-second draw frames | ~9 | **82** | **9x** |

### Theoretical vs Measured

| Component | Theoretical | Measured (writeBytes) |
|-----------|-------------|----------------------|
| SPI pixel transfer | 1.1 ms | — |
| setAddrWindow | 0.4 ms | — |
| CPU computation | 0.08 ms | — |
| **Total** | **~1.6 ms** | **~2.3 ms** |

The ~0.7 ms gap between theoretical and measured is attributed to:
- Per-chunk loop overhead in `writeColorFast` (32-pixel batches, not continuous DMA)
- SPI FIFO busy-wait between chunks
- Function call overhead across 105 drawing primitives per frame

## Approaches Tested

| Approach | Result | Notes |
|----------|--------|-------|
| `SPI.writePattern(pattern, 2, 50000)` | **Crash** | Crashes at ~2/3 screen fill (~50K pixel boundary) |
| `SPI.writePattern(pattern, 2, 32)` | **Crash** | Small chunks still crash on ESP8266 |
| `SPI.writeBytes(buf, 64)` in 32-pixel chunks | **Works** | 2,342 us/dial — final implementation |
| `SPI.write16(color)` per-pixel loop | **Works** | 3,802 us/dial — stable fallback |

`writePattern` crashes are likely due to ESP8266 SPI FIFO state conflicts when called within an ILI9341 transaction context. `writeBytes` and `write16` both work correctly because they handle the FIFO differently (writeBytes fills and flushes atomically; write16 does single 16-bit atomic writes).

## Summary

The SPI fast path eliminates the `delay(1)` bottleneck that consumed 98.5% of ESP8266 rendering time. Using `SPI.writeBytes()` with 64-byte FIFO batches, dial rendering improved from 64.4ms to 2.3ms — a **27.5x speedup**. The ESP8266 SPI bus is now utilized at approximately 48% efficiency (vs under 2% before), with the remaining overhead split between `setAddrWindow` commands, per-chunk loop overhead, and CPU computation.
