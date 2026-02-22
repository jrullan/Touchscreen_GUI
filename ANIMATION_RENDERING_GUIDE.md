# Achieving Smooth Animation on ILI9341 with Arduino

## The Fundamental Constraint

The ILI9341 is a **framebuffer-less SPI display**. It has internal RAM (172,800 bytes for 240x320x18-bit) but no compositing engine. Every pixel you want to change must be pushed over the SPI bus. The question of smooth animation on this hardware reduces to one problem: **how many bytes per second can you push over SPI, and how do you minimize the number you need to send?**

### SPI Bandwidth Budget

| SPI Clock | Raw Throughput | Full-Screen Fills/sec (240x320 RGB565) |
|-----------|---------------|----------------------------------------|
| 20 MHz | 2.5 MB/s | ~16 FPS |
| 40 MHz | 5.0 MB/s | ~32 FPS |
| 80 MHz | 10.0 MB/s | ~65 FPS |

A full-screen write is 153,600 bytes (240 x 320 x 2). At 40 MHz SPI, you get ~32 full-screen paints per second — enough for smooth animation **if** you could saturate the bus with pixel data. In practice, the current Touchscreen_GUI architecture wastes most of that bandwidth on redundant draws, per-primitive SPI overhead, and CPU-bound rendering. Below is how to fix each layer.

---

## Layer 1: Eliminate SPI Transaction Overhead

### The Problem

Today, every call to `fillRect()`, `drawLine()`, `drawCircle()`, etc. through Adafruit_ILI9341 performs:

```
SPI.beginTransaction()       // ~2-5 us
digitalWrite(CS, LOW)        // ~1-2 us
Send command (0x2A)          // Column address set (5 bytes)
Send command (0x2B)          // Row address set (5 bytes)
Send command (0x2C)          // Memory write (1 byte)
Send pixel data              // 2 * width * height bytes
digitalWrite(CS, HIGH)       // ~1-2 us
SPI.endTransaction()         // ~1-2 us
```

That is **~11 bytes of overhead + 6-12 us of GPIO/SPI setup** per primitive. A Dial needle update calls `fillTriangle` (erase) + `fillCircle` (erase) + `fillTriangle` (draw) + `fillCircle` (draw) = 4 transactions. A Trend update with 32 points calls `drawLine` 64 times (erase + redraw) = 64 transactions. The overhead alone costs **~400-800 us** before any pixels move.

### The Fix: Batch Under startWrite/endWrite

Adafruit_GFX already supports this — `IconButton.cpp:82-95` is the only place in the library that uses it. The pattern should be applied everywhere:

```cpp
void Dial::drawNeedleAndValue() {
    myCanvas->tft->startWrite();          // Open SPI transaction ONCE

    drawNeedle(x, y, previousValue, ..., bgColor);  // Erase old
    drawNeedle(x, y, currentValue, ..., color);      // Draw new
    // value text drawing here too

    myCanvas->tft->endWrite();            // Close SPI transaction ONCE
}
```

Inside a `startWrite()` / `endWrite()` block, all `writeFillRect()`, `writePixel()`, `writeFastHLine()` etc. skip the per-call CS toggle and transaction setup. This eliminates the overhead entirely for multi-primitive draws.

**Apply this to:** `Dial::drawNeedleAndValue()`, `Trend::drawValues()`, `Terminal::update()`, `Gauge::drawFill()`, `Button::show()`, `Numkey::drawFrame()`, `Buttongrid::drawGrid()`.

**Expected gain:** 30-50% faster rendering for widgets with multiple primitives per update.

---

## Layer 2: Minimize Pixel Count with Address Windowing

### The Problem

The erase-then-redraw pattern sends every pixel twice. A 50x50 button update fills 2,500 pixels in bgColor, then draws text over it (more pixels). A Gauge redraws both the empty and filled regions every update, even if the bar moved by 1 pixel.

### The Fix: Update Only the Changed Strip

The ILI9341's address window (CASET 0x2A / RASET 0x2B) lets you target an arbitrary rectangle. Instead of redrawing the full widget, calculate the minimal changed region.

**Gauge example — only redraw the delta:**

```cpp
void Gauge::drawFill() {
    int newTop = map(currentValue, scaleMin, scaleMax, h - borderWidth, borderWidth);
    int oldTop = map(previousValue, scaleMin, scaleMax, h - borderWidth, borderWidth);

    if (newTop == oldTop) return;  // Nothing changed

    if (newTop < oldTop) {
        // Bar grew upward — fill the new strip in fgColor
        tft->fillRect(x + borderWidth, y + newTop,
                      w - 2 * borderWidth, oldTop - newTop, fgColor);
    } else {
        // Bar shrank — clear the vacated strip in bgColor
        tft->fillRect(x + borderWidth, y + oldTop,
                      w - 2 * borderWidth, newTop - oldTop, bgColor);
    }
    previousValue = currentValue;
}
```

This changes the Gauge update from **2 full-height fillRects** to **1 small-strip fillRect**. If the bar moved 2 pixels on a 100px-tall gauge, you send 2 * barWidth * 2 bytes instead of 2 * barWidth * 100 bytes — a **50x reduction**.

**Apply the same principle to:** Slider (same bar logic), Dial (only erase the old needle arc, not the whole dial), Display (only clear the text bounding box if text length changed).

---

## Layer 3: Use a Scanline or Strip Buffer

### The Problem

Adafruit_GFX's `drawLine()` and `fillTriangle()` send pixels one at a time or in thin horizontal runs. Each run requires setting an address window. For diagonal lines, this means one address window per scanline.

### The Fix: Render to a RAM Strip, Then Blast

Allocate a small buffer in RAM (1-10 scanlines of the animated region), render into it using in-memory pixel operations, then send the entire strip in one SPI burst.

```cpp
// Strip buffer: 1 scanline of a 100px-wide animated region = 200 bytes
#define STRIP_W 100
uint16_t stripBuf[STRIP_W];

void renderDialStrip(int y, uint16_t* buf, int w) {
    // Fill background
    memset(buf, bgColor16, w * 2);  // Fast memory fill

    // Draw needle pixels into buffer (no SPI, just RAM writes)
    // ... calculate which pixels of the needle fall on this scanline ...
    // buf[needleX] = needleColor;

    // Blast the entire strip in one SPI write
    tft->startWrite();
    tft->setAddrWindow(dialX, y, w, 1);
    tft->writePixels(buf, w);   // Adafruit_SPITFT supports this
    tft->endWrite();
}

// Animate: render only the strip rows that the needle occupies
for (int y = needleMinY; y <= needleMaxY; y++) {
    renderDialStrip(y, stripBuf, STRIP_W);
}
```

**Why this is fast:**
- `memset` + in-memory pixel writes are ~100x faster than SPI per byte
- One `setAddrWindow()` + one `writePixels()` per scanline = minimal SPI overhead
- Only the rows containing the animated element are updated

**RAM cost:** 200 bytes for a 100px-wide strip at RGB565. Even an Arduino Mega can afford this.

For ESP32 with more RAM, you can buffer the entire animated widget region:

```cpp
// Buffer a 100x100 dial area = 20,000 bytes (ESP32 has 520KB)
uint16_t dialBuf[100 * 100];
// Render dial into buffer, then:
tft->startWrite();
tft->setAddrWindow(dialX, dialY, 100, 100);
tft->writePixels(dialBuf, 10000);
tft->endWrite();
```

This sends 20,000 bytes in a single SPI burst — at 40 MHz, that takes **~4ms**. You could update this region at **250 FPS** from a pure SPI bandwidth perspective.

---

## Layer 4: Use ILI9341 Hardware Vertical Scroll

### The Problem

`Terminal::scroll()` clears the entire terminal area with `fillRect()`, shifts text line buffers in RAM, then redraws all lines. For a 10-line terminal, that is 1 `fillRect` + 10 `drawString` calls, each touching hundreds of characters worth of pixels.

### The Fix: ILI9341 Vertical Scroll Feature

The ILI9341 has built-in hardware scrolling. It shifts its internal RAM contents vertically **with zero SPI pixel transfer**. You only need to send 3 small commands:

```cpp
// One-time setup: define scroll area
void setupScroll(int topFixed, int scrollHeight, int bottomFixed) {
    tft->startWrite();
    tft->writeCommand(0x33);                    // Vertical Scroll Definition
    tft->write16(topFixed);                     // Top Fixed Area (rows)
    tft->write16(scrollHeight);                 // Scroll Area height (rows)
    tft->write16(bottomFixed);                  // Bottom Fixed Area (rows)
    tft->endWrite();
}

// Each scroll step: shift by N rows
void scrollBy(int rows) {
    static int scrollOffset = 0;
    scrollOffset = (scrollOffset + rows) % scrollAreaHeight;

    tft->startWrite();
    tft->writeCommand(0x37);                    // Vertical Scroll Start Address
    tft->write16(scrollOffset);                 // New start row
    tft->endWrite();
}
```

**After calling `scrollBy(lineHeight)`:**
- The display hardware shifts all rows up by `lineHeight` pixels — **instantly, no SPI pixel data**
- The vacated rows at the bottom still contain old pixel data
- You only need to draw the **one new line** into the vacated area

This transforms Terminal scrolling from "clear everything + redraw 10 lines" to "send 4 bytes of scroll command + draw 1 line." For a Trend widget, the same technique shifts the entire chart left, and you only draw the newest data point column.

**Constraints:** Hardware scroll only works vertically and wraps around the 320-row (or 240-row in landscape) display RAM. The scroll area must be defined as a contiguous row range. This maps naturally to Terminal and Trend widgets.

---

## Layer 5: Use DMA for Non-Blocking SPI Transfers

### The Problem

Today, all SPI transfers are blocking — the CPU sits in a loop sending bytes and cannot do anything else (compute next frame, read touch input, run application logic) until the transfer completes.

### The Fix: Platform-Specific DMA

**ESP32** — The ESP32 SPI peripheral has built-in DMA. The ESP-IDF `spi_device_transmit()` and the Arduino-ESP32 SPI library support DMA transactions. The Adafruit_ILI9341 library for ESP32 can be configured to use DMA:

```cpp
// ESP32: SPI DMA channel configuration
// Adafruit_ILI9341 on ESP32 uses SPI DMA internally for large transfers
// The key is to use writePixels() with large buffers to trigger DMA path
tft->startWrite();
tft->setAddrWindow(x, y, w, h);
tft->writePixels(buffer, w * h);  // DMA transfer if buffer > threshold
tft->endWrite();
// CPU is free during the DMA transfer
```

**STM32** — The existing `Adafruit_ILI9341_STM` library already uses DMA (referenced in `GUI_TFT.h:17`). This is already in the codebase but only activated for STM32 targets.

**Arduino AVR** — No DMA available. The SPI peripheral is CPU-driven. The only optimization is maximizing the SPI clock (up to 8 MHz on ATmega2560) and minimizing the number of bytes sent (Layers 1-4).

**Double-buffer with DMA** — On ESP32/STM32, combine a strip buffer with DMA for true pipelining:

```
Frame N:
  CPU renders strip N+1 into Buffer B  ←→  DMA sends Buffer A to display
  (parallel — CPU and SPI bus both busy)

Frame N+1:
  CPU renders strip N+2 into Buffer A  ←→  DMA sends Buffer B to display
```

This hides the SPI transfer time behind CPU rendering time. With two 240-byte strip buffers (1 scanline each), the overhead is just 480 bytes of RAM.

---

## Layer 6: Dirty Rectangles Without a Full Framebuffer

You do not need a full-screen framebuffer to implement dirty-rectangle tracking. Add a `dirty` flag and bounding box to each widget:

```cpp
class Widget {
    // ... existing members ...
    bool dirty = true;
    lv_area_t dirtyArea;  // {x1, y1, x2, y2} of changed region
};
```

Then change `Canvas::scan()` to only update dirty widgets:

```cpp
void Canvas::updateWidgets() {
    for (int i = 0; i < widgets.count(); i++) {
        if (widgets[i]->visible && widgets[i]->dirty) {
            widgets[i]->update();
            widgets[i]->dirty = false;
        }
    }
}
```

And change `Canvas::redraw()` (called when a Popup/Numkey hides) to use region overlap:

```cpp
void Canvas::redrawRegion(int rx, int ry, int rw, int rh) {
    for (int i = 0; i < widgets.count(); i++) {
        Widget* w = widgets[i];
        if (w->visible &&
            w->x < rx + rw && w->x + w->w > rx &&
            w->y < ry + rh && w->y + w->h > ry) {
            w->show();
        }
    }
}
```

This eliminates the single most expensive operation in the current codebase — the full-screen repaint triggered by modal close.

---

## Putting It All Together: Animated Dial Example

Here is what a smooth-animation Dial update would look like combining all layers:

```cpp
#define DIAL_SIZE 100
uint16_t stripBuf[DIAL_SIZE];  // 200 bytes RAM

void Dial::animateNeedle(int targetValue) {
    int step = (targetValue > currentValue) ? 1 : -1;

    while (currentValue != targetValue) {
        currentValue += step;

        // Calculate the bounding box of old + new needle position
        int minY = min(oldNeedleTipY, newNeedleTipY) - 2;
        int maxY = max(oldNeedleTipY, newNeedleTipY) + 2;

        tft->startWrite();  // One SPI transaction for entire frame

        // Render only the affected scanlines
        for (int y = minY; y <= maxY; y++) {
            // Render this scanline into RAM buffer
            renderDialScanline(y, stripBuf, DIAL_SIZE);

            // Blast scanline to display
            tft->setAddrWindow(dialX, y, DIAL_SIZE, 1);
            tft->writePixels(stripBuf, DIAL_SIZE);
        }

        tft->endWrite();  // Close SPI transaction

        // ~20 scanlines * 200 bytes = 4,000 bytes per frame
        // At 40 MHz SPI: ~0.8ms per frame = 1000+ FPS potential
    }
}
```

**Cost per animation frame:**
- CPU: render ~20 scanlines into 200-byte buffer (~50 us on ESP32)
- SPI: send ~4,000 bytes (~0.8ms at 40 MHz)
- **Total: ~1ms per frame — well above 60 FPS for the animated region**

---

## Summary: Impact of Each Layer

| Layer | Technique | Pixel Savings | Implementation Effort |
|-------|-----------|---------------|----------------------|
| 1 | startWrite/endWrite batching | 0% fewer pixels, 30-50% less overhead | Low — wrap existing draw calls |
| 2 | Delta-only address windowing | 50-95% fewer pixels for Gauge/Slider | Medium — per-widget logic |
| 3 | Scanline/strip buffer | Enables fast composited rendering | Medium — new render path |
| 4 | ILI9341 hardware scroll | ~99% fewer pixels for Terminal/Trend scroll | Low — 3 ILI9341 commands |
| 5 | DMA SPI transfers | 0% fewer pixels, CPU freed for next frame | Medium — platform-specific |
| 6 | Dirty rectangle tracking | Skip unchanged widgets entirely | Low — flag + overlap test |

### What's Achievable

| Scenario | Current FPS (est.) | After Optimizations (est.) |
|----------|-------------------|---------------------------|
| Static screen, no changes | N/A (still redraws) | 0 SPI traffic (dirty flags) |
| Single Dial needle sweep | ~5-10 FPS | 60+ FPS (strip buffer) |
| Trend chart scrolling | ~3-5 FPS | 30+ FPS (hardware scroll + 1 column draw) |
| Terminal text scroll | ~5-8 FPS | 30+ FPS (hardware scroll + 1 line draw) |
| Gauge bar animation | ~10-15 FPS | 60+ FPS (delta strip only) |
| Full-screen transition | ~2-3 FPS | ~15-30 FPS (DMA + strip buffer) |
| Popup close (redraw behind) | ~2 FPS (redraws all) | ~10-20 FPS (region overlap) |

The ILI9341 hardware is not the bottleneck — the SPI bus at 40 MHz can push 32 full screens per second. The bottleneck is the software sending far more pixels than necessary and paying per-primitive overhead. Fixing the software layer unlocks the hardware's actual capability.
