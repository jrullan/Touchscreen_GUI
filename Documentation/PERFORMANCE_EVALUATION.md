# Performance Evaluation

## 1. Execution Overhead

### 1.1 Touch-to-Response Latency

The touch input pipeline has two stacked timing gates that create significant lag.

**`Canvas::scan()` gates at 100ms** (`Canvas.h:60`, `SCAN_SAMPLING_TIME`), and inside it `getTouchedPoint()` gates again at 25ms (`TOUCH_SAMPLING_TIME`). Combined worst-case path from finger-down to event handler firing:

```
100ms scan gate + 25ms touch gate + touch read + map() calls + widget iteration + handler
= 130-150ms minimum latency (8-9 frames at 60fps)
```

**Recommendation:** Reduce `SCAN_SAMPLING_TIME` to 16-25ms. The 25ms touch gate is redundant when scan already throttles; unify into a single gate.

---

### 1.2 Default Debounce Is Too Aggressive

`Button` and `OptionButton` default to **500ms** debounce (`Button.cpp:75`, `OptionButton.cpp:40`). This locks out all further touch on that widget for half a second after each press.

**Recommendation:** Reduce default debounce to 100-150ms. The current 500ms was likely set to compensate for noisy resistive touch, but 150ms is sufficient for debounce on any controller.

---

### 1.3 Touch Driver Overhead Varies Widely

| Driver | Per-read Cost | Notes |
|--------|--------------|-------|
| **XPT2046** | ~6 SPI transfers, interrupt-driven, 3ms throttle | Best performer. Smart `besttwoavg()` uses bitshift division. |
| **FT6206** | 2 full I2C transactions (16 bytes), polled | Blocking. `readData()` called unconditionally on every `getPoint()` with no caching. |
| **STMPE610** | SPI register reads in a **busy-wait FIFO drain loop** | `getPoint()` (`Adafruit_STMPE610.cpp:340-359`) calls `while(!bufferEmpty()) readData()` — unbounded blocking. |
| **Seeedstudio** | 12 `analogRead()` calls per axis + floating-point pressure math | Extreme oversampling. Also has bugs: `y = (samples[0]+samples[0])` doubles sample 0 instead of averaging. |

**Recommendations:**
- **FT6206:** Cache the last `readData()` result and only re-read when `touched()` state changes.
- **STMPE610:** Limit the FIFO drain loop to a fixed iteration count (e.g., 10) to prevent unbounded blocking.
- **Seeedstudio:** Fix the averaging bugs. Replace floating-point pressure calculation with fixed-point integer math. Reduce oversampling from 12 to 4-6 reads.

---

### 1.4 Widget Hit-Testing in Grids

`Buttongrid::checkTouch()` and `Numkey::checkTouch()` iterate rows x columns with nested loops to find which cell was pressed. A 4x4 grid does up to 16 boundary comparisons per touch.

**Recommendation:** Replace with direct index calculation:
```cpp
int col = (p->x - x - borderWidth) / btnWidth;
int row = (p->y - y - borderWidth) / btnHeight;
int id  = row * columns + col + 1;
```
This is O(1) instead of O(rows * cols).

---

### 1.5 Coordinate Mapping Cost

Every touch event runs 2-4 `map()` calls, each performing a multiply and a divide. On AVR (no hardware divider), integer division is ~50-100 cycles.

**Recommendation:** Pre-calculate scale factors at init time:
```cpp
// At init:
float xScale = (float)(outMax - outMin) / (inMax - inMin);
// At runtime:
p.x = (int)((rawX - inMin) * xScale) + outMin;  // multiply only, no divide
```

---

### 1.6 Rendering Bottlenecks

Almost every widget uses an **erase-then-redraw** pattern: draw the old content in `bgColor`, then draw new content in `fgColor`. This doubles the number of TFT draw calls per update.

| Widget | Update Cost | Pattern |
|--------|------------|---------|
| **Dial** | 2x `fillTriangle` + 2x `fillCircle` per needle move | Erase old needle in bgColor, draw new |
| **Trend** | ~64 `drawLine` calls per single value added | Full erase + full redraw of all points |
| **Terminal** | Full `fillRect` clear + redraw all lines | Even if only one new line was added |
| **Gauge** | 2x `fillRect` per update | Redraws both empty and filled bar regions |
| **Display** | `fillRect` + `drawString` every update | Even if text hasn't changed |
| **IconButton** | Pixel-by-pixel bitmap write | 4,096 individual `writePixel()` calls for 64x64 icon |
| **Slider** | 2x `fillRect` per update | No change detection |

**Recommendations:**

- **Trend:** Replace the O(n) array shift with a **circular buffer** (ring buffer). Track a write index and wrap around — zero memory shifting. Only erase and redraw the newest line segment rather than the entire history.

- **Terminal:** Only redraw the newly added line. On scroll, use `memcpy()` instead of character-by-character loops (`Terminal.cpp:143-148`).

- **Dial:** Track `previousAngle` and only redraw if the needle actually moved by at least 1 pixel in angle.

- **Display/Gauge/Slider:** Add a dirty flag. Skip `update()` entirely when the value hasn't changed. The Dial already does this check (`Dial.cpp:199`) but most other widgets do not.

- **IconButton:** Use `drawRGBBitmap()` (Adafruit GFX block write) instead of per-pixel `writePixel()` for non-transparent icons.

---

### 1.7 Canvas `redraw()` Is a Full Repaint

`Canvas::redraw()` (`Canvas.cpp:244-255`) calls `show()` on **every visible widget**. It is triggered by `Numkey.hide()` and `Popup.hide()`, meaning closing a modal repaints the entire screen.

**Recommendation:** Implement dirty-rectangle invalidation. When a modal closes, only repaint widgets whose bounding boxes overlap the modal's area:
```cpp
void Canvas::redrawRegion(int rx, int ry, int rw, int rh) {
    for (int i = 0; i < widgets.count(); i++) {
        if (widgets[i]->visible && overlaps(widgets[i], rx, ry, rw, rh))
            widgets[i]->show();
    }
}
```

---

### 1.8 Drawing During Touch Handlers

`Buttongrid::checkTouch()` calls `fillRect()` and `drawLabel()` **inside the touch event handler** (`Buttongrid.cpp:212-214`). TFT draws block the SPI bus for milliseconds, delaying the return of `scan()` and stalling the main loop.

**Recommendation:** Set a dirty flag in the touch handler; defer the actual draw to the next `update()` cycle.

---

## 2. Binary Size

### 2.1 Per-Widget Memory Footprint (RAM)

| Type | Instance Size | Heap Allocs | Total per Instance |
|------|--------------|-------------|-------------------|
| Widget (base) | 44 bytes | — | 44 bytes |
| Button | 65 bytes | 9 bytes (Text) | ~78 bytes |
| Numkey | 65+ bytes | 7 bytes (text buf) | ~76 bytes |
| Buttongrid (4x4) | 65+ bytes | 144 bytes (labels + states) | ~213 bytes |
| Display | ~49 bytes | 9 bytes (Text) | ~62 bytes |
| Terminal | ~60 bytes | line buffers | ~200+ bytes |
| Dial | ~100 bytes | 9 bytes (buf) | ~113 bytes |
| Gauge | ~100 bytes | 9 bytes (buf) | ~113 bytes |
| Trend | ~100 bytes | 33 bytes (values + buf) | ~137 bytes |
| Slider | ~100 bytes | 9 bytes (buf) | ~113 bytes |
| Canvas | 148 bytes | StackArray growth | ~170 bytes |

**Typical application** (10 Buttons + 2 Dials + 1 Trend + 1 Terminal + Canvas):
~1,500-2,000 bytes RAM for widget instances alone.

This is reasonable for ESP32 (520KB SRAM) but tight on Arduino Mega (8KB).

---

### 2.2 StackArray Never Shrinks

`StackArray.h` doubles capacity on overflow (2 -> 4 -> 8 -> 16 -> ...) but **never shrinks** (`StackArray.h:203-205` — shrink logic is commented out). If a screen temporarily holds 20 widgets then drops to 5, the array stays at 32 slots (128 bytes for `Widget*`).

**Recommendation:** Implement `shrinkToFit()` or at minimum halve when usage drops below 25%. Alternatively, switch to a fixed-capacity array sized at compile time via a template parameter, since widget counts are known at design time.

---

### 2.3 Unused Touch Buffer Wastes 40 Bytes

`Canvas.h:108-110` declares `xTouchBuffer[5]` and `yTouchBuffer[5]` (40 bytes total) plus `touchBufferIndex`. These are **never read or written anywhere** in the codebase — dead code from an incomplete averaging feature.

**Recommendation:** Remove the unused buffers, or implement the intended coordinate averaging to improve touch accuracy.

---

### 2.4 `Widget::getType()` If-Chain

`Widget.h:93-123` uses a 15-branch if/else-if chain returning string literals. This compiles to ~300-400 bytes of comparison and branch instructions.

**Recommendation:** Replace with a lookup table. Since type codes aren't contiguous, use a small map or switch (which the compiler can optimize to a jump table).

---

### 2.5 `icons.h` Is 112KB

`icons.h` contains raw bitmap data for demo icons. If `IconButton` is used and this header is included, it adds **112KB to program memory**.

**Recommendation:** Move icon data to per-project sketch files rather than bundling in the library. The library should only provide the `drawRGBABitmap()` rendering function; actual bitmap data belongs to the application.

---

### 2.6 All Canvas Subclasses Compilable Simultaneously

There are no guards preventing inclusion of multiple `Canvas_*.h` files. Each pulls in its respective touch driver library. If a user accidentally includes more than one, unused driver code and I2C/SPI library code inflates the binary.

**Recommendation:** Add compile-time selection via a single define:
```cpp
#define TGUI_TOUCH_DRIVER TGUI_XPT2046
#include <Canvas.h>  // Automatically includes only the selected driver
```

---

### 2.7 Virtual Dispatch Overhead

The Widget hierarchy uses 3 virtual methods (`show`, `update`, `checkTouch`) across 12+ subclasses. Each class gets a vtable (~12 bytes code), and each instance carries a 4-byte vtable pointer.

This is an acceptable tradeoff for the polymorphic design and only adds ~200-400 bytes total. No change recommended — the flexibility justifies the cost.

---

### 2.8 Text Class Heap Fragmentation

Every Button allocates `new Text(8)` in its constructor (`Button.cpp:24`), creating many small (9-byte) heap allocations. With 20+ buttons, this creates 20+ `malloc` calls with per-allocation overhead of 4-8 bytes each.

**Recommendation:** Use a fixed inline `char[8]` buffer inside Text instead of `malloc`. For widgets that never change text at runtime, this eliminates heap fragmentation entirely:
```cpp
class Text {
    char text[9];  // Inline, no malloc
    unsigned char _textSize;
};
```

---

## 3. Bugs Found During Review

| Location | Bug |
|----------|-----|
| `Seeedtouch.cpp:159` | `y = (samples[0]+samples[0])` — doubles sample[0] instead of adding sample[1] |
| `Seeedtouch.cpp:127` | `x = (samples[0] + samples[1])` — sums but doesn't average (missing `/2`) |
| `Canvas.h:108-110` | Touch buffer arrays declared, initialized, never used |
| `Popup.cpp:210` | `update()` is empty (`return;`) — popup can never incrementally update |

---

## 4. Prioritized Recommendations

### Quick Wins (minimal code changes, immediate impact)

| # | Change | Impact |
|---|--------|--------|
| 1 | Reduce `SCAN_SAMPLING_TIME` from 100ms to 20ms | Cuts touch latency by ~80ms |
| 2 | Reduce default button debounce from 500ms to 150ms | Doubles effective responsiveness |
| 3 | Remove unused touch buffer arrays | Frees 44 bytes RAM |
| 4 | Add dirty flag to Display, Gauge, Slider `update()` | Eliminates redundant redraws |
| 5 | Fix Seeedstudio averaging bugs | Correct touch coordinates |

### Medium Effort (targeted refactors)

| # | Change | Impact |
|---|--------|--------|
| 6 | Replace Trend array shift with circular buffer | O(1) insert vs O(n) shift |
| 7 | Terminal: only redraw changed line, use `memcpy` for scroll | ~90% fewer draw calls on scroll |
| 8 | Buttongrid/Numkey: direct index calc instead of nested loops | O(1) vs O(rows*cols) hit test |
| 9 | Pre-calculate coordinate scale factors at init | Eliminates per-touch division |
| 10 | Inline Text buffer instead of malloc | Eliminates heap fragmentation |

### Architecture Improvements (larger refactors)

| # | Change | Impact |
|---|--------|--------|
| 11 | Dirty-rectangle invalidation on `Canvas::redraw()` | Only repaint overlapping widgets |
| 12 | Defer TFT draws out of touch handlers | Unblocks scan loop during interaction |
| 13 | Compile-time touch driver selection | Prevents unused driver code in binary |
| 14 | Move icon bitmap data out of library headers | Saves up to 112KB flash |
| 15 | Cap STMPE610 FIFO drain loop iterations | Prevents unbounded blocking |
