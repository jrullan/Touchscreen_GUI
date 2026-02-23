# Theoretical Dial Performance Analysis — ESP8266

## Hardware Specifications

| Parameter | Value |
|-----------|-------|
| MCU | ESP8266EX @ 80 MHz (12.5 ns/cycle) |
| SPI clock | HSPI @ 40 MHz (25 ns/bit) |
| Display | ILI9341, 240x320, RGB565 (16-bit/pixel) |
| SPI FIFO | 64 bytes |
| DMA | Not used by Adafruit library on ESP8266 |

## Dial Widget Configuration (from demo)

| Parameter | Value |
|-----------|-------|
| radius | 50 px |
| borderWidth | 5 px |
| tickSize | 10 px |
| gap | 5 px |
| needleR | radius - tickSize - gap = **35 px** |
| Hub radius | 4 px |
| scaleMin / scaleMax | 60 / 90 |
| fontSize | 2 (12x16 px per char) |
| showVal | true |

## The Dominant Bottleneck: `delay(1)` in `writeColor`

The single most important finding from examining the Adafruit library source is this code in `Adafruit_SPITFT.cpp` (lines 1399-1411):

```cpp
// ESP8266 path inside writeColor():
if (connection == TFT_HARD_SPI) {
#if defined(ESP8266)
    do {
      uint32_t pixelsThisPass = len;
      if (pixelsThisPass > 50000)
        pixelsThisPass = 50000;
      len -= pixelsThisPass;
      delay(1); // <-- Called on EVERY writeColor invocation!
      while (pixelsThisPass--) {
        hwspi._spi->write(hi);
        hwspi._spi->write(lo);
      }
    } while (len);
```

Every call to `writeColor()` — whether writing 768 pixels or 4 — executes `delay(1)`. This is a watchdog/yield mechanism specific to the ESP8266 port, and it dominates all rendering cost.

Additionally, pixel data is written byte-by-byte via `SPI.write(hi); SPI.write(lo);` — two separate 8-bit SPI transfers per pixel, rather than bulk/DMA writes. Compare this to the ESP32 path which uses a 32-pixel temp buffer with `writePixels()` and no delay.

### What `delay(1)` actually costs

On ESP8266, `delay(ms)` loops calling `optimistic_yield()` until `millis()` has incremented by at least `ms`. Since `millis()` has 1 ms resolution:

- If called just before a tick: ~0-100 us wait
- If called just after a tick: ~900-1000 us wait
- **First call in a burst: ~500 us average**
- **Subsequent back-to-back calls: ~990 us each** (the SPI work between calls is only ~5-10 us, so the next `delay(1)` must wait for the full next tick)

Over N consecutive `writeColor` calls: approximately **N milliseconds total**.

## Call Chain Analysis

Every drawing primitive ultimately reaches `writeColor`:

```
fillTriangle -> writeFastHLine -> writeFillRect -> writeFillRectPreclipped
                                                   -> setAddrWindow()
                                                   -> writeColor()  <- delay(1)!

fillCircle -> writeFastVLine -> writeFillRect -> writeFillRectPreclipped -> ...
           -> fillCircleHelper -> writeFastVLine -> ... -> writeColor()

drawChar (transparent, size=2) -> writeFillRect per set pixel -> ... -> writeColor()

fillRect -> startWrite + writeFillRectPreclipped + endWrite -> writeColor()
```

### ILI9341 `setAddrWindow` optimization

The `Adafruit_ILI9341::setAddrWindow` implementation caches the last column/row addresses (static locals `old_x1/x2/y1/y2`). When coordinates haven't changed, it skips the corresponding SPI command. For fillTriangle scanlines (changing Y each time but X range varies), PASET is always sent but CASET may sometimes be cached.

## Counting `writeColor` Calls Per Dial Frame

### 1. Text value erase — `fillRect(48 x 16 = 768 pixels)`

One call to `writeFillRectPreclipped` -> **1 `writeColor` call**

### 2. Text value draw — `drawNumber` (transparent mode, size=2)

From the Adafruit default font (`glcdfont.c`), set pixel counts per digit:

| Digit | Font bytes | Set pixels |
|-------|-----------|------------|
| 0 | 3E 51 49 45 3E | 19 |
| 1 | 00 42 7F 40 00 | 10 |
| 2 | 72 49 49 49 46 | 16 |
| 3 | 21 41 49 4D 33 | 15 |
| 4 | 18 14 12 7F 10 | 14 |
| 5 | 27 45 45 45 39 | 17 |
| 6 | 3C 4A 49 49 31 | 16 |
| 7 | 41 21 11 09 07 | 11 |
| 8 | 36 49 49 49 36 | 17 |
| 9 | 46 49 49 29 1E | 16 |

For the dial range 60-90 (always 2 digits):
- Tens digit (6,7,8,9): avg **15.0** set pixels
- Units digit (0-9): avg **15.1** set pixels
- **Total: ~30 `writeFillRect` calls -> 30 `writeColor` calls**

### 3. Erase old needle — `drawNeedle(previousValue, bgColor)`

**fillTriangle scanline count** depends on the Y-span of the triangle at each angle. The needle sweeps from 225 deg (value=60) through 90 deg/top (value=75) to 315 deg (value=90). Computing the triangle vertex Y-span for representative values:

| Value | Degree | Y-span (scanlines) |
|-------|--------|-------------------|
| 60 | 585 (225 deg) | 27 |
| 68 | 513 (153 deg) | 18 |
| 75 | 450 (90 deg) | 35 |
| 82 | 387 (27 deg) | 18 |
| 90 | 315 (315 deg) | 27 |

**Average: ~24 scanlines per fillTriangle -> 24 `writeColor` calls**

**fillCircle(r=4):** Tracing through the midpoint circle algorithm in `Adafruit_GFX.cpp`:
- `fillCircle` calls `writeFastVLine` (1 call) + `fillCircleHelper` (both corners)
- `fillCircleHelper` with r=4 iterates 3 times, drawing 4 `writeFastVLine` calls per iteration
- **Total: 1 + 12 = 13 `writeFastVLine` calls -> 13 `writeColor` calls**

Needle erase total: 24 + 13 = **37 `writeColor` calls**

### 4. Draw new needle — `drawNeedle(currentValue, color)`

Same geometry: **37 `writeColor` calls**

### Total `writeColor` calls per frame

| Operation | Calls |
|-----------|-------|
| Text erase (fillRect) | 1 |
| Text draw (drawNumber, transparent) | 30 |
| Erase old needle (fillTriangle) | 24 |
| Erase old hub (fillCircle r=4) | 13 |
| Draw new needle (fillTriangle) | 24 |
| Draw new hub (fillCircle r=4) | 13 |
| **Total** | **105** |

## Theoretical Time Breakdown

### A. `delay(1)` dead time — the dominant cost

105 calls, each synchronizing to the next `millis()` tick. The first call averages ~500 us, subsequent calls wait ~990 us each (since the inter-call SPI work is only ~5-10 us):

**Total delay: ~500 + 104 x 990 = ~103,500 us = ~104 ms**

### B. SPI pixel transfer

At 40 MHz with byte-by-byte `SPI.write()` — each byte takes ~400-500 ns (200 ns wire time + register/polling overhead):

| Operation | Pixels | Bytes | Time (us) |
|-----------|--------|-------|-----------|
| Text erase (48x16) | 768 | 1,536 | 691 |
| Text draw (30 x 2x2) | 120 | 240 | 108 |
| 2x fillTriangle (~24 scanlines, ~3px avg width) | 144 | 288 | 130 |
| 2x fillCircle (13 lines, ~6px avg width) | 156 | 312 | 140 |
| **Total** | **1,188** | **2,376** | **~1,069** |

**Total SPI pixel transfer: ~1.1 ms**

### C. `setAddrWindow` overhead

Per call: 3 `writeCommand` + 4 `SPI_WRITE16` + DC toggles + function calls = ~4.0-4.5 us.
With ILI9341 column/row caching, average = ~3.5 us.

105 calls x 3.5 us = **~368 us = ~0.4 ms**

### D. CPU computation

| Operation | Time (us) |
|-----------|-----------|
| 4x fastSin/fastCos LUT lookups | 2 |
| 4x map/constrain | 2 |
| Vertex computation (integer mult/shift) | 2 |
| fillTriangle sort + Bresenham stepping (2x) | 20 |
| fillCircle midpoint algorithm (2x) | 10 |
| drawChar font read + loop (2 chars) | 40 |
| **Total CPU** | **~76** |

### E. SPI session overhead

With the v2 nesting counter in `GUI_TFT.h`, only 1 `SPI.beginTransaction`/`endTransaction` pair per `Dial::update()`:

**~10 us**

## Theoretical Total Per Drawing Frame

| Component | Time | % of total |
|-----------|------|------------|
| `delay(1)` dead time | **~104 ms** | **98.5%** |
| SPI pixel transfer | ~1.1 ms | 1.0% |
| setAddrWindow | ~0.4 ms | 0.4% |
| CPU computation | ~0.08 ms | 0.1% |
| SPI session | ~0.01 ms | 0.0% |
| **Total** | **~105.6 ms** | **100%** |

## Theoretical vs Measured

The demo slider sweeps 0-100 in steps of 2 (`incrementTimer = 10ms`). The dial value is `map(slider, 0, 100, 60, 90)`, so the dial changes by 1 only every other slider step (~50% of frames). On non-drawing frames, `_dirty` is false and `update()` returns immediately (~10 us).

| Metric | Theoretical | Measured |
|--------|-------------|---------|
| Per drawing frame | ~106 ms | ~88 ms* |
| Active frame ratio | ~50% | ~50% |
| Average across all frames | ~53 ms | 44.1 ms |

*Estimated from: 44.1ms avg x 91 frames / ~46 active frames = 87ms

The theoretical estimate is ~20% higher than measured. The most likely explanation is that `delay(1)` on ESP8266 averages slightly less than 1ms per call — depending on timer interrupt alignment, `optimistic_yield` short-circuits, and the watchdog servicing pattern, the effective average may be closer to 0.8ms per call. At 0.83ms average: 105 x 0.83 = 87ms, matching the measured value almost exactly.

## Theoretical Maximum Without `delay(1)`

If `writeColor` on ESP8266 omitted the `delay(1)` call (as it does on other platforms):

| Component | Time |
|-----------|------|
| SPI pixel transfer | 1.1 ms |
| setAddrWindow | 0.4 ms |
| CPU computation | 0.08 ms |
| SPI session | 0.01 ms |
| **Total** | **~1.6 ms** |

That would yield **~625 FPS theoretical** for just the dial, or over **60x faster** than current performance. The ESP8266's rendering is almost entirely `delay(1)` dead time.

For comparison, the ESP32 `writeColor` path uses a 32-pixel temp buffer with `writePixels()` and **no delay**, so an ESP32 running this same code would approach this theoretical ~1.6ms figure (modified by its own SPI clock and DMA capabilities).

## Summary

The ESP8266 dial performance is fundamentally limited by a single line in the Adafruit library: `delay(1)` inside `writeColor()`, called once per drawing primitive (scanline, pixel block, or fill rectangle). With 105 such calls per dial frame, approximately **98.5% of the rendering time is spent in `delay(1)` — not in SPI transfer, not in computation**. The actual SPI bus utilization is under 2%.

This is an architectural limitation of the ESP8266 Adafruit port, not of the ILI9341 or SPI hardware. The `delay(1)` exists to feed the ESP8266's watchdog timer and prevent WiFi stack starvation on long fills, but it penalizes all fills equally regardless of size.
