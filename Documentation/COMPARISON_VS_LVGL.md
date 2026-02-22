# Touchscreen GUI vs LVGL — Comparative Analysis

## At a Glance

| Aspect | Touchscreen GUI | LVGL (v9.6) |
|--------|----------------|-------------|
| **Language** | C++ (Arduino) | C (C++ compatible) |
| **License** | MIT | MIT |
| **Widgets** | 12 | 30+ |
| **Min RAM** | ~2 KB usable | ~32 KB (+ display buffer) |
| **Min Flash** | ~10-15 KB (est.) | ~128 KB |
| **Display drivers** | ILI9341 only | ILI9341, ST7789, SSD1306, SDL2, DRM, and many more |
| **Touch drivers** | 4 hardcoded (XPT2046, FT6206, STMPE610, Seeedstudio) | Any, via callback abstraction |
| **Rendering** | Immediate-mode, direct to SPI display | Retained-mode, draw-buffer with dirty rectangles |
| **Layout engine** | Manual x,y placement | Flexbox + CSS Grid |
| **Event system** | Single function-pointer per widget | Multi-listener, 128+ event types, bubbling/trickling |
| **Animation** | None | Timeline, easing curves, chained sequences |
| **Styling/Themes** | Per-widget color properties | Centralized style system with state-based selectors + themes |
| **Multi-display** | No | Yes (unlimited) |
| **Multi-input** | No | Yes (pointer, keypad, encoder, button) |
| **OS integration** | None (bare-loop) | FreeRTOS, Zephyr, RT-Thread, POSIX, custom |
| **Build system** | Arduino library / PlatformIO | CMake, Make, PlatformIO, ESP-IDF, Zephyr, Arduino |
| **GPU acceleration** | None | DMA2D, VG-Lite, PXP, OpenGL ES, NanoVG, and more |
| **Project maturity** | 0.1 beta (2015), single author | v9.6, 500+ contributors, corporate backing |

---

## 1. Architecture

### Touchscreen GUI

A thin layer over `Adafruit_ILI9341` and `Adafruit_GFX`. Widgets draw directly to the SPI display using the GFX primitives (`fillRect`, `drawLine`, `drawCircle`, etc.). There is no intermediate draw buffer — every pixel write goes straight to hardware over SPI.

```
Application sketch
  └─ Widget.show() / Widget.update()
       └─ Adafruit_ILI9341 (SPI)
            └─ ILI9341 display
```

Touch input is polled in `Canvas::scan()` and dispatched to widgets via `checkTouch()`. The canvas owns a flat `StackArray<Widget*>` and iterates it linearly.

### LVGL

A full retained-mode rendering engine. Objects exist in a parent-child tree. When a property changes, LVGL marks the affected screen region as invalid. On the next refresh cycle, only those dirty rectangles are re-rendered into an off-screen draw buffer, which is then flushed to the display via a user-provided callback.

```
Application code
  └─ lv_obj tree (retained scene graph)
       └─ lv_refr (dirty-rect compositor)
            └─ Draw buffer (RAM)
                 └─ flush_cb() (user-provided, SPI/DMA/parallel)
                      └─ Display hardware
```

This means LVGL never sends redundant pixels to the display, whereas Touchscreen GUI redraws entire widget regions regardless of what actually changed.

**Verdict:** LVGL's architecture is fundamentally more efficient for complex UIs. Touchscreen GUI's architecture is simpler and easier to understand, which is an advantage for learning and for very simple UIs with few widgets.

---

## 2. Rendering Performance

### Touchscreen GUI

- **No dirty tracking.** `Canvas::redraw()` calls `show()` on every visible widget. Closing a Popup or Numkey triggers a full-screen repaint.
- **Erase-then-redraw pattern.** Most widgets (Dial, Trend, Terminal, Gauge) draw old content in `bgColor` then redraw in `fgColor` — doubling SPI traffic.
- **Trend widget** shifts an array O(n) and redraws all ~32 line segments (64 draw calls) per single value added.
- **No draw buffer.** Every `fillRect` / `drawLine` is an individual SPI transaction to the ILI9341. On a 240x320 display at 16-bit color, a full-screen clear is 153,600 bytes over SPI.
- **IconButton** renders bitmaps pixel-by-pixel (`writePixel` in a loop) instead of block transfer.

### LVGL

- **Dirty rectangle invalidation.** Only changed screen regions are re-rendered. Multiple dirty areas are merged to reduce draw passes (`lv_refr_join_area()`).
- **Draw buffer.** Rendering happens into RAM first, then the buffer is flushed in one DMA-capable block transfer. Buffer can be 1/10 screen size (PARTIAL mode), full screen (DIRECT mode), or even triple-buffered.
- **Draw task batching.** Individual draw operations (fill, border, text, image) are queued as tasks and dispatched to the software renderer or a GPU unit. Multiple draw units can run in parallel on multi-core MCUs.
- **Hardware acceleration.** Optional GPU backends (STM32 DMA2D, NXP VG-Lite/PXP, OpenGL ES) offload rendering from the CPU entirely.
- **Optimized primitives.** Font rendering uses glyph caching. Image rendering uses decoded image caching. Shadow and circle outlines are cached.

**Verdict:** LVGL is dramatically more efficient for any UI with more than a few widgets. The dirty-rectangle system alone eliminates the majority of redundant SPI traffic that dominates Touchscreen GUI's rendering cost. Touchscreen GUI's direct-to-SPI approach only wins in the degenerate case of a single widget on screen where the overhead of maintaining a draw buffer is unnecessary.

---

## 3. Input Handling

### Touchscreen GUI

- Touch polled at 100ms intervals (`SCAN_SAMPLING_TIME`), with a secondary 25ms gate in `getTouchedPoint()`.
- Worst-case touch-to-handler latency: **130-150ms**.
- Default button debounce: **500ms** (overly conservative).
- Each touch chipset requires its own `Canvas_*` subclass with hardcoded calibration constants and coordinate mapping.
- Only supports touch (pointer). No keyboard, encoder, or hardware button input.
- No gesture recognition (no swipe, pinch, long-press, scroll).
- Widget hit-testing for grids uses O(rows*cols) nested loops.

### LVGL

- Input devices polled via a configurable timer (default 33ms, user-settable).
- Built-in gesture recognition: click, double-click, triple-click, long press, long press repeat, scroll with momentum, swipe.
- Supports four input types: pointer (touch/mouse), keypad, encoder (rotary knob), hardware buttons mapped to screen coordinates.
- Unlimited input devices, each independently configured.
- Touch driver is fully abstracted behind a read callback — no library-level driver code. The user provides a function that returns coordinates and press state.
- Hit testing uses the object tree and clipping areas, not brute-force iteration.

**Verdict:** LVGL's input system is far more capable and responsive. Touchscreen GUI's fixed polling intervals and hardcoded driver model limit both responsiveness and portability.

---

## 4. Widget Ecosystem

### Touchscreen GUI (12 widgets)

| Category | Widgets |
|----------|---------|
| Input | Button, Numkey, Buttongrid, Popup, OptionButton, IconButton, Slider |
| Display | Display, Terminal |
| Indicator | Dial, Gauge, Trend |

All widgets have fixed visual appearances. Customization is limited to colors, border width, font size, and corner radius. No layout engine — all positions are absolute x,y coordinates.

### LVGL (30+ widgets)

| Category | Widgets |
|----------|---------|
| Basic | Button, Label, Image, ImageButton, LED, Line, Arc |
| Input | Textarea, Slider, Switch, Checkbox, Dropdown, Roller, SpinBox, Keyboard, ButtonMatrix |
| Data | Chart, Table, Meter, Scale, Calendar, Bar, ProgressBar |
| Container | List, Menu, Tabview, Tileview, Window, MessageBox, Canvas |
| Advanced | AnimImage, GIF, Lottie, Span, QR Code, Barcode |

All widgets support the full style system (background, border, shadow, outline, text, opacity — all state-dependent). Flexbox and Grid layout engines handle automatic positioning, sizing, and responsive reflow.

**Verdict:** LVGL offers 2.5x more widget types, and each widget is far more customizable through the style system. Touchscreen GUI covers the basics for simple HMI dashboards but lacks the building blocks for complex interfaces.

---

## 5. Event System

### Touchscreen GUI

Each interactive widget holds a single function pointer:
```cpp
void (*eventHandler)(Button*);
```
One handler per widget. No event types — the handler must check `widget->touched` to determine state. No event propagation between parent and child.

### LVGL

Multiple handlers per object, filtered by event type:
```c
lv_obj_add_event_cb(obj, handler, LV_EVENT_CLICKED, user_data);
lv_obj_add_event_cb(obj, handler2, LV_EVENT_VALUE_CHANGED, user_data);
```
128+ event types including input events, drawing hooks, lifecycle events, and custom user events. Events bubble up the object tree (parent receives child events) or trickle down. A Subject-Observer data binding system allows widgets to auto-update when shared data changes.

**Verdict:** LVGL's event system is production-grade and comparable to desktop GUI frameworks. Touchscreen GUI's single-function-pointer model works for simple cases but breaks down as UI complexity grows.

---

## 6. Memory Footprint

### Touchscreen GUI

- **Per Button:** ~78 bytes (65 instance + 9 heap for Text + malloc overhead)
- **Canvas:** ~170 bytes
- **Typical small app** (10 Buttons + 2 Dials + 1 Trend + Canvas): **~1.5-2 KB RAM**
- **Flash:** ~10-15 KB for core library code (excluding Adafruit_GFX/ILI9341 dependencies)
- No draw buffer needed (direct SPI writes)

### LVGL

- **Per base object (`lv_obj_t`):** ~50 bytes minimum (grows with styles, children, special attributes)
- **Minimum system:** ~32 KB RAM (heap pool + display buffer)
- **Flash:** ~128 KB minimum (core + one render backend + basic widgets)
- Display buffer required: 1/10 screen (~15 KB for 320x240 RGB565) to full screen (~150 KB)

### Comparison

| Scenario | TGUI RAM | LVGL RAM |
|----------|----------|----------|
| 5 Buttons + 1 Display | ~600 bytes | ~32+ KB |
| 20 Buttons + 5 Indicators + Trend | ~3 KB | ~40+ KB |
| Minimum viable app | ~500 bytes | ~32 KB |

**Verdict:** Touchscreen GUI uses 15-60x less RAM for equivalent widget counts. This is its strongest advantage. On an Arduino Mega with 8 KB SRAM, LVGL simply cannot run, while Touchscreen GUI fits comfortably. On ESP32 (520 KB SRAM), both fit easily and LVGL's rendering efficiency more than compensates for its higher baseline.

---

## 7. Portability & Hardware Abstraction

### Touchscreen GUI

- **Display:** Tightly coupled to `Adafruit_ILI9341`. Supporting a different display controller (ST7789, SSD1306, etc.) would require rewriting `GUI_TFT` and potentially every widget's drawing code.
- **Touch:** Four hardcoded Canvas subclasses, each with baked-in calibration constants and coordinate transforms. Adding a new touch controller means writing a new `Canvas_*` class.
- **MCU:** Works on any platform that supports Arduino + Adafruit GFX, but pin definitions and SPI configuration are scattered across `#ifdef` blocks in multiple files.

### LVGL

- **Display:** Fully abstracted. The user provides a `flush_cb` function that accepts a pixel buffer and area coordinates. The library never touches SPI, I2C, or any bus directly. Any display works — SPI, parallel, HDMI, SDL2 desktop simulator.
- **Touch:** Fully abstracted. The user provides a `read_cb` function returning coordinates and press state. Any touch controller works without library changes.
- **MCU:** Runs on bare-metal, FreeRTOS, Zephyr, RT-Thread, Linux, Windows, macOS. OSAL (OS Abstraction Layer) handles threading, mutexes, and timers.

**Verdict:** LVGL is hardware-agnostic by design. Touchscreen GUI is locked to ILI9341 + one of four touch controllers. Porting Touchscreen GUI to a new display requires invasive changes; porting LVGL requires writing two callback functions.

---

## 8. Developer Experience

### Touchscreen GUI

**Strengths:**
- Simple, readable code. A beginner can understand the entire library in an afternoon.
- Minimal boilerplate. Create widget → set properties → add to canvas → call `scan()` in loop.
- 14 example sketches, each demonstrating a single widget.
- Zero configuration. No config file to set up.
- Immediate visual feedback — widgets render directly, no buffering abstraction to understand.

**Weaknesses:**
- No layout engine. Every widget needs manual x,y positioning. Changing screen size requires repositioning everything by hand.
- No documentation generator. API docs are in one markdown file and inline comments.
- No simulator. Must flash to hardware to test UI changes.

### LVGL

**Strengths:**
- Professional documentation site with interactive examples.
- Desktop simulator (SDL2) allows UI development without hardware.
- SquareLine Studio — commercial drag-and-drop UI editor that generates LVGL code.
- Flexbox/Grid layout eliminates manual coordinate math.
- Active community (15,000+ GitHub stars, Discord, forum).

**Weaknesses:**
- Steep learning curve. The configuration file (`lv_conf.h`) has 600+ lines of options.
- Abstractions add cognitive load (draw buffers, flush callbacks, tick integration, timer handler).
- Debugging rendering issues requires understanding the layer/task/buffer pipeline.
- Larger API surface (hundreds of functions) takes time to learn.

**Verdict:** Touchscreen GUI wins for simplicity and approachability, especially for Arduino hobbyists. LVGL wins for professional development workflows with its simulator, editor tooling, and layout system.

---

## 9. Where Each Library Fits

### Use Touchscreen GUI when:

- Running on **very low-RAM MCUs** (Arduino Mega, ATmega2560 with 8 KB SRAM)
- Building **simple dashboards** with a handful of indicators and buttons
- The display is specifically an **ILI9341** with one of the four supported touch controllers
- The developer is an **Arduino beginner** learning embedded GUI concepts
- The project is a **quick prototype** where development speed matters more than polish
- Total widget count stays **under ~15-20**

### Use LVGL when:

- The MCU has **32+ KB RAM** (ESP32, STM32F4+, RP2040, any modern ARM Cortex-M)
- Building **complex multi-screen applications** with navigation, forms, data visualization
- **Multiple display types** or touch controllers need to be supported
- The project needs **animations, gestures, scrolling, or themes**
- **Long-term maintainability** matters (active development, large community)
- A **desktop simulator** is wanted for faster UI iteration
- The application needs to be **portable** across hardware platforms
- **Professional polish** is required (anti-aliased fonts, shadows, gradients, smooth transitions)

---

## 10. What Touchscreen GUI Could Learn from LVGL

These are the highest-value architectural ideas from LVGL that could be adopted by Touchscreen GUI without abandoning its lightweight nature:

1. **Dirty-flag rendering.** Even without a draw buffer, each widget could track a `dirty` boolean and skip `update()` entirely when nothing changed. This is a trivial change that eliminates the majority of redundant SPI traffic.

2. **Callback-based hardware abstraction.** Replace the four `Canvas_*` subclasses with a single Canvas that takes a `readTouch` function pointer at construction time. This makes adding new touch controllers a one-function exercise instead of a new class.

3. **Region-scoped redraw.** When a Popup or Numkey hides, pass its bounding rectangle to `redraw()` and only repaint widgets that overlap that region — instead of repainting everything.

4. **Circular buffer for Trend.** Replace the O(n) array shift with a ring buffer and a write index. Only draw the newest line segment instead of erasing and redrawing the entire history.

5. **Unified timing gate.** Merge the 100ms scan gate and 25ms touch gate into a single configurable interval (e.g., 20ms). This alone would cut touch latency by 80ms.
