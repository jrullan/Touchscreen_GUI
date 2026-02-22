# Touch Functionality Evaluation

## Current Architecture

Touchscreen_GUI supports four touch controllers through a pattern where each controller gets its own `Canvas_*` subclass:

```
Canvas (abstract)
  ├─ Canvas_XPT2046      → XPT2046_Touchscreen (SPI, resistive)
  ├─ Canvas_FT6206        → Adafruit_FT6206 (I2C, capacitive, multi-touch capable)
  ├─ Canvas_STMPE610      → Adafruit_STMPE610 (SPI or I2C, resistive)
  └─ Canvas_SEEEDTOUCH    → SeeedstudioTouch (4-wire analog, resistive)
```

The user selects a controller at compile time by instantiating the corresponding Canvas subclass:

```cpp
Canvas_XPT2046 canvas(TFT_PORTRAIT, BLACK, TFT_CS, TFT_DC, TS_CS);
```

A `Touch` abstract class exists with three virtual methods (`begin`, `getPoint`, `touched`), and each driver implements it. However, the actual coordinate mapping, calibration, and orientation logic lives in the `Canvas_*` subclass rather than behind the `Touch` interface. This means the `Touch` abstraction alone is not sufficient to swap controllers — you must also swap the Canvas subclass.

---

## Problems

### 1. Touch and Display Are Fused Into One Class

Each `Canvas_*` subclass creates **both** the TFT display object and the touch driver object in its `init()`. The Canvas is simultaneously the display manager, the touch manager, and the widget container. This means:

- You cannot pair a new touch controller with the existing display logic without writing a full new Canvas subclass.
- You cannot test touch independently from display.
- Adding a 5th touch controller requires duplicating all the display initialization boilerplate from an existing Canvas subclass.

Every `Canvas_*.h` file contains nearly identical code for `init()` (create TFT, fill screen, set orientation, init timers) with only the touch object creation and `getTouchedPoint()` differing.

### 2. Calibration Is Hardcoded at Compile Time

Each controller has its own set of `#define` constants for coordinate mapping:

| Controller | Constants | Location |
|------------|-----------|----------|
| XPT2046 | `XPT2046_MIN_X=3894`, `MAX_X=261`, `MIN_Y=3891`, `MAX_Y=320` | `Canvas_XPT2046.h` |
| STMPE610 | `STMPE_MINX=240`, `MAXX=3775`, `MINY=240`, `MAXY=3700` | `Adafruit_STMPE610.h` |
| Seeedstudio | `TS_MINX=232`, `MAXX=1780`, `MINY=166`, `MAXY=1826` | `Seeedtouch.h` |
| FT6206 | None (returns screen coordinates directly) | — |

These values are specific to individual display+touch panel combinations. A different ILI9341 breakout board with the same XPT2046 chip will likely have different calibration ranges. There is no way to set calibration values at runtime or run a calibration routine. Users must edit the `#define` values and recompile.

### 3. Orientation Handling Is Duplicated and Inconsistent

Each `Canvas_*::getTouchedPoint()` implements its own orientation transformation with different approaches:

**XPT2046** — swaps x/y, then uses `map()` with different parameter order for portrait vs landscape:
```cpp
int oldX = p.x; p.x = p.y; p.y = oldX;  // Swap axes
if (_mode == TFT_PORTRAIT) {
    p.x = map(p.x, XPT2046_MIN_X, XPT2046_MAX_X, 0, w);
    p.y = map(p.y, XPT2046_MIN_Y, XPT2046_MAX_Y, 0, h);
} else {
    oldX = p.x;
    p.x = map(p.y, XPT2046_MAX_Y, XPT2046_MIN_Y, 0, w);
    p.y = map(oldX, XPT2046_MIN_X, XPT2046_MAX_X, 0, h);
}
```

**FT6206** — no `map()` needed, but applies different arithmetic per orientation:
```cpp
if (tft->layoutMode == TFT_PORTRAIT) {
    p.x = 240 - p.x;
    p.y = 320 - p.y;
}
if (tft->layoutMode == TFT_LANDSCAPE) {
    int oldX = p.x;
    p.x = p.y;
    p.y = 240 - oldX;
}
```

**STMPE610** — maps to fixed 240x320, then applies orientation:
```cpp
p.x = map(p.x, STMPE_MINX, STMPE_MAXX, 0, 240);
p.y = map(p.y, STMPE_MINY, STMPE_MAXY, 0, 320);
if (_mode == TFT_LANDSCAPE) {
    int oldX = p.x;
    p.x = 320 - p.y;
    p.y = 240 - oldX;
}
if (_mode == TFT_PORTRAIT) {
    p.x = 240 - p.x;
}
```

**Seeedstudio** — uses `map()` with `tft->width()/height()`, swaps axes differently for landscape.

Four different implementations of essentially the same operation: map raw coordinates to screen pixels accounting for orientation. The `Point::rotate()` method exists with six rotation modes but is never called from any Canvas subclass.

### 4. Only Two Orientations Supported

The code handles `TFT_PORTRAIT` and `TFT_LANDSCAPE` but not the inverted variants (portrait-180, landscape-180). The `Point` class defines `POINT_PORTRAIT2` and `POINT_LANDSCAPE2` constants, suggesting this was planned but never completed.

### 5. Dual Timing Gates Create Excessive Latency

`Canvas::scan()` gates on `SCAN_SAMPLING_TIME` (100ms). Inside it, `getTouchedPoint()` gates again on `TOUCH_SAMPLING_TIME` (25ms). These are independent timers that stack:

- Best case: touch happens right at both timer expirations → ~0ms latency
- Worst case: touch happens right after scan timer fires → 100ms wait for next scan + potential 25ms touch gate = **125ms**
- Average case: **~60-75ms** latency

For comparison, a responsive touch UI targets 16-33ms (30-60 FPS input polling).

### 6. No Touch Filtering or Smoothing

The `Canvas.h` declares `xTouchBuffer[5]` and `yTouchBuffer[5]` but they are **never populated or read** — this is dead code. Each touch event uses a single raw coordinate sample (after the driver's own internal filtering).

The XPT2046 driver has good internal filtering (`besttwoavg()` — takes 3 measurements, averages the 2 closest). The FT6206 relies on the chip firmware. The STMPE610 does 4-sample hardware averaging. But the Seeedstudio driver has bugs in its averaging (`y = samples[0] + samples[0]` doubles sample 0 instead of summing samples 0 and 1).

There is no library-level filtering to smooth noisy touch coordinates across successive reads — jitter on resistive panels will pass directly to widget hit-testing.

### 7. No Touch Release Detection

The `Touch::touched()` method returns whether the screen is currently pressed, and `Canvas::scan()` only processes events when `ts->touched()` returns true. There is no tracking of press-to-release transitions. Widgets handle this internally — `Button::checkTouch()` toggles `touched = !touched` on each registered press — but the Canvas layer has no concept of a touch lifecycle (press → move → release).

This means:
- No drag/swipe detection is possible at the framework level.
- The Slider widget must be tapped repeatedly; it cannot be dragged smoothly.
- No gesture recognition (long-press, double-tap, pinch on FT6206).

### 8. FT6206 Multi-Touch Is Ignored

The FT6206 capacitive controller reads up to 2 simultaneous touch points (`touchX[2]`, `touchY[2]`) and reports touch count via `touches`. However, `Canvas_FT6206::getTouchedPoint()` only reads touch point 0. The second touch point is discarded.

### 9. SPI Bus Contention Is Unmanaged

Both XPT2046 and the ILI9341 display share the hardware SPI bus. Each uses `SPI.beginTransaction()` / `SPI.endTransaction()` independently, which prevents data corruption. However, there is no coordination to avoid polling the touch controller while a display write is in progress. On high-frequency updates, a touch read could insert itself between display SPI transactions, adding latency to rendering.

### 10. Platform Pin Definitions Are Scattered

Touch pin assignments for the Seeedstudio analog touch are defined via `#ifdef` blocks across `Canvas.h` for each MCU platform (STM32, ESP32, ATmega, etc.). The STMPE610 chip-select pin is similarly defined per platform in `Adafruit_STMPE610.h`. This scattering makes it difficult to support a new board — you must edit multiple header files.

---

## Suggested Improvements

### A. Decouple Touch from Canvas

Separate the touch system into its own standalone component that Canvas uses via composition, not inheritance. The core idea: the user creates a touch driver independently and passes it to Canvas.

```cpp
// Touch driver interface (expanded from current Touch.h)
class TouchDriver {
public:
    virtual bool begin() = 0;
    virtual bool touched() = 0;
    virtual Point readRaw() = 0;      // Raw coordinates from hardware
    virtual uint8_t touchCount() { return touched() ? 1 : 0; }  // Multi-touch
};

// Calibration + orientation applied by a shared mapper
struct TouchCalibration {
    int rawMinX, rawMaxX;
    int rawMinY, rawMaxY;
    bool swapXY;
    bool invertX;
    bool invertY;
};

class TouchMapper {
public:
    TouchMapper(TouchDriver* driver, TouchCalibration cal);
    void setOrientation(uint8_t orientation);  // 0, 90, 180, 270
    void setScreenSize(int w, int h);
    Point read();          // Returns screen-mapped coordinates
    bool pressed();
private:
    TouchDriver* _driver;
    TouchCalibration _cal;
    uint8_t _orientation;
    int _screenW, _screenH;
};
```

Then Canvas takes a `TouchMapper*` instead of owning the touch system:

```cpp
// User code — touch driver is independent of canvas
XPT2046Driver touch(TS_CS_PIN);
TouchCalibration cal = { 3894, 261, 3891, 320, true, false, false };
TouchMapper mapper(&touch, cal);

Canvas canvas(TFT_PORTRAIT, BLACK, TFT_CS, TFT_DC, &mapper);
```

**Benefits:**
- Adding a new touch controller means writing one small class that implements `TouchDriver` (3 methods). No Canvas changes needed.
- Calibration values are data, not `#define` constants. They can be stored in EEPROM, read from a config file, or determined by a runtime calibration routine.
- Orientation transformation is written once in `TouchMapper`, not four times across Canvas subclasses.
- Canvas subclasses (`Canvas_XPT2046`, etc.) can be eliminated entirely. One Canvas class works with any touch controller.
- Display vendors ship different panels with different raw coordinate ranges but the same controller IC. Runtime calibration handles this naturally.

### B. Add Runtime Calibration

Provide a built-in 3-point calibration routine that the user can call at startup or on demand:

```cpp
TouchCalibration cal = mapper.calibrate();  // Shows targets on screen, user taps each
// Optionally save to EEPROM:
EEPROM.put(0, cal);
```

The calibration routine displays crosshairs at 3 known screen positions, records the raw coordinates the user taps, and computes the affine transformation matrix. This is the standard approach used by resistive touchscreen systems and handles:

- Different raw coordinate ranges per panel
- Axis swaps and inversions
- Non-uniform scaling (trapezoidal distortion on resistive panels)
- All four orientations from a single calibration

The 3-point affine calibration uses 6 coefficients (a-f) to map raw (rx, ry) to screen (sx, sy):

```
sx = a*rx + b*ry + c
sy = d*rx + e*ry + f
```

This replaces all the per-driver `map()` calls and orientation `if/else` blocks with a single matrix multiply. The matrix coefficients change when orientation changes — just recompute from the stored calibration points.

### C. Track Touch State Transitions

Add press/move/release tracking to the touch layer:

```cpp
enum TouchState { IDLE, PRESSED, HELD, RELEASED };

class TouchMapper {
    // ... existing ...
    TouchState state();
    Point pressPoint();       // Where the press began
    Point currentPoint();     // Current position (valid during HELD)
    unsigned long pressDuration();  // How long held so far
    Point dragDelta();        // currentPoint - pressPoint
};
```

The mapper polls the driver and maintains a state machine:

```
IDLE ──(touched)──→ PRESSED ──(still touched, next poll)──→ HELD
  ↑                                                           │
  └──────────────(released)────── RELEASED ←──(released)──────┘
```

This enables:
- **Drag** — Slider becomes draggable by reading `currentPoint()` during HELD state
- **Long-press** — Trigger after `pressDuration()` exceeds threshold
- **Swipe** — Detect from `dragDelta()` direction and speed on RELEASED
- **Tap vs drag discrimination** — If `dragDelta()` < threshold on RELEASED, it's a tap

Widget event handlers receive richer information:

```cpp
void buttonHandler(Button* btn, TouchEvent event) {
    if (event.state == PRESSED) { /* visual feedback */ }
    if (event.state == RELEASED) { /* action */ }
}
```

This also fixes the current `touched = !touched` toggle in `Button::checkTouch()`, which is fragile — if a touch is detected twice in quick succession (debounce failure), the toggle gets out of sync.

### D. Unified Timing — Single Poll Interval

Replace the dual timing gates (100ms scan + 25ms touch) with a single configurable interval:

```cpp
canvas.setPollInterval(20);  // 20ms = 50 Hz input polling
```

Inside `scan()`, a single `millis()` check gates all touch processing:

```cpp
bool Canvas::scan() {
    if (millis() - _lastPoll < _pollInterval) return false;
    _lastPoll = millis();

    _mapper->poll();  // Read hardware + update state machine

    if (_mapper->state() == PRESSED || _mapper->state() == HELD) {
        Point p = _mapper->currentPoint();
        if (inBounds(&p)) {
            routeToWidgets(&p, _mapper->state());
        }
    }
    if (_mapper->state() == RELEASED) {
        routeRelease();
    }
    return true;
}
```

Default interval: 20ms (50 Hz). This cuts average latency from ~65ms to ~10ms — a 6x improvement that is immediately perceptible to the user.

### E. Coordinate Filtering

Implement the touch buffer that is already declared but unused. A simple moving-average or median filter over the last N readings smooths jitter on resistive panels:

```cpp
// Median-of-3 filter (good jitter rejection, 3 samples)
Point TouchMapper::readFiltered() {
    Point samples[3];
    for (int i = 0; i < 3; i++) samples[i] = _driver->readRaw();

    // Median of each axis independently
    int x = median3(samples[0].x, samples[1].x, samples[2].x);
    int y = median3(samples[0].y, samples[1].y, samples[2].y);

    return applyCalibration(x, y);
}
```

For capacitive controllers (FT6206) which are already smooth, the filter can be bypassed or set to depth 1. Make it configurable:

```cpp
mapper.setFilterDepth(3);  // 3 for resistive, 1 for capacitive
```

### F. Support All Four Orientations

The affine calibration matrix from improvement B handles all orientations automatically. When `setOrientation()` is called, recompute the matrix coefficients. No per-driver `if/else` blocks needed. The four orientations become a rotation applied to the calibration matrix:

```
0°:   sx = a*rx + b*ry + c,   sy = d*rx + e*ry + f
90°:  sx = screenW - sy_0,     sy = sx_0
180°: sx = screenW - sx_0,     sy = screenH - sy_0
270°: sx = sy_0,               sy = screenH - sx_0
```

### G. Expose Multi-Touch on Capable Controllers

For the FT6206 (and any future capacitive controller), expose the second touch point:

```cpp
class TouchDriver {
    virtual uint8_t touchCount() { return touched() ? 1 : 0; }
    virtual Point readRaw(uint8_t index = 0) = 0;
};

// FT6206 implementation
uint8_t FT6206Driver::touchCount() { return min(touches, 2); }
Point FT6206Driver::readRaw(uint8_t n) { return Point(touchX[n], touchY[n], 1); }
```

Even if the widget system only uses touch 0, exposing the data allows advanced users to implement pinch-to-zoom or two-finger gestures in their application code.

### H. Make Touch Drivers External

Stop bundling the Adafruit_FT6206, Adafruit_STMPE610, and XPT2046_Touchscreen source files inside the library. Instead, make them external dependencies. Each is already an independent Arduino library. The Touchscreen_GUI library would only define the `TouchDriver` interface; users install the controller library they need and write a thin adapter:

```cpp
#include <XPT2046_Touchscreen.h>
#include <TouchDriver.h>

class XPT2046Adapter : public TouchDriver {
    XPT2046_Touchscreen _ts;
public:
    XPT2046Adapter(uint8_t cs) : _ts(cs) {}
    bool begin() override { return _ts.begin(); }
    bool touched() override { return _ts.touched(); }
    Point readRaw() override {
        TS_Point p = _ts.getPoint();
        return Point(p.x, p.y, p.z);
    }
};
```

**Benefits:**
- The library shrinks significantly (no bundled driver source).
- Users get the latest driver versions from their library managers.
- Supporting a new controller never requires a Touchscreen_GUI release — the user writes a 10-line adapter.
- No risk of version conflicts with other projects using the same driver libraries.

### I. Decouple Touch SPI from Display SPI

On boards with multiple SPI buses (ESP32 has VSPI + HSPI), allow the touch controller to run on a separate bus:

```cpp
XPT2046Adapter touch(TS_CS, SPI1);  // Touch on HSPI
Canvas canvas(TFT_CS, TFT_DC, SPI);  // Display on VSPI
```

This eliminates SPI bus contention entirely — touch reads never interrupt display writes, and both can operate concurrently with DMA.

Even on single-SPI systems, awareness of bus sharing should be explicit. The current code works because Arduino's `SPI.beginTransaction()` serializes access, but there is no documentation or safeguard around this.

---

## Summary

| Problem | Severity | Fix |
|---------|----------|-----|
| Touch fused into Canvas subclasses | High | A. Decouple into TouchDriver + TouchMapper |
| Hardcoded calibration constants | High | B. Runtime calibration with affine matrix |
| No press/move/release tracking | High | C. Touch state machine |
| 100ms+ input latency | High | D. Single 20ms poll interval |
| No coordinate filtering (buffer unused) | Medium | E. Configurable median/average filter |
| Only 2 of 4 orientations | Medium | F. Matrix-based orientation (follows from B) |
| FT6206 multi-touch ignored | Low | G. Expose touch index in driver interface |
| Bundled driver source files | Medium | H. External dependencies + adapter pattern |
| SPI bus contention | Low | I. Support separate SPI buses |
| Seeedstudio averaging bugs | Medium | Fix `samples[0]+samples[0]` typo |
| Point::rotate() never called | Low | Remove dead code or integrate via matrix |
| Scattered platform pin #ifdefs | Medium | Move to user-provided config or constructor args |

The single highest-impact change is **A + B + D together**: decouple touch from Canvas, add runtime calibration, and cut the poll interval. This transforms the touch system from a rigid, high-latency, 4-controller-only design into a portable, responsive, any-controller architecture — while actually reducing the total code in the library by eliminating four nearly-identical Canvas subclasses.
