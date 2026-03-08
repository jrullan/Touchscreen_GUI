# Touchscreen_GUI — Memory & Flash Analysis

## Date: February 2026

## Platform

| Parameter | Value |
|-----------|-------|
| MCU | ESP8266EX (WEMOS D1 Mini) @ 80 MHz |
| Total DRAM | 81,920 bytes |
| Flash | 4 MB |
| Board core | esp8266:esp8266@3.1.2 |
| Compiler | xtensa-lx106-elf-gcc |

ESP8266 memory segments:
- **DATA** — initialized global/static variables (copied from flash to RAM at boot)
- **RODATA** — read-only constants, string literals, `const` arrays (loaded into RAM on ESP8266 unless `PROGMEM`)
- **BSS** — zero-initialized global/static variables (RAM)
- **IROM** — program code (runs from flash via instruction cache)

**Static RAM = DATA + RODATA + BSS**. This is the fixed RAM cost visible at compile time. The remaining RAM (81,920 − static RAM) is available as heap for `malloc`, `new`, stack, and system overhead (~2–3 KB for ESP8266 WiFi/system internals even when WiFi is off).

---

## Compile-Time Measurements

All measurements taken with `arduino-cli compile --fqbn esp8266:esp8266:d1_mini`. Values in bytes.

### Layer-by-Layer Build-Up

| Configuration | DATA | RODATA | BSS | Static RAM | IROM (Flash) |
|---------------|------|--------|-----|------------|--------------|
| Bare ESP8266 (empty sketch + Serial) | 1,496 | 920 | 25,656 | 28,072 | 235,588 |
| + Guitft (TFT driver only) | 1,504 | 920 | 26,304 | 28,728 | 247,856 |
| + Canvas_Touch + TouchDriver + TouchMapper | 1,504 | 948 | 26,648 | 29,100 | 251,412 |

### Incremental Framework Costs

| Layer | Δ RAM | Δ Flash | What it adds |
|-------|-------|---------|-------------|
| Guitft (Adafruit_ILI9341 subclass) | +656 | +12,268 | TFT driver, SPI fast path, GFX primitives |
| Canvas_Touch + touch system | +372 | +3,556 | Canvas, TouchMapper, TouchDriver_XPT2046, StackArray, Point |

**Framework base cost: ~1,028 bytes RAM + ~15,824 bytes flash** above a bare ESP8266 sketch.

### Per-Widget Sample Measurements

Each sample uses Canvas_Touch as the base (29,100 bytes static RAM, 251,412 bytes flash).

| Sample | Widgets | Static RAM | IROM (Flash) | Δ RAM | Δ Flash |
|--------|---------|------------|--------------|-------|---------|
| TGUI_Sample (base) | Canvas only | 29,100 | 251,412 | — | — |
| TGUI_Sample_Button | 1 Button | 29,308 | 255,828 | +208 | +4,416 |
| TGUI_Sample_Display | 1 Display | 29,252 | 252,820 | +152 | +1,408 |
| TGUI_Sample_Terminal | 1 Terminal | 29,368 | 253,508 | +268 | +2,096 |
| TGUI_Sample_Numkey | 1 Numkey | 29,408 | 258,996 | +308 | +7,584 |
| TGUI_Sample_IconButton | 1 IconButton + 2 icons | 36,516 | 256,372 | +7,416 | +4,960 |
| TGUI_Sample_Dial | 1 Dial + 2 Buttons | 29,508 | 259,796 | +408 | +8,384 |
| TGUI_Sample_Slider | 3 Sliders + 4 Displays | 29,748 | 255,492 | +648 | +4,080 |
| TGUI_Sample_Screens | 2 Screens + 2 Buttons + 2 Displays | 29,576 | 257,412 | +476 | +6,000 |
| Benchmark (full demo) | Dial + 3 Sliders + 4 Displays + Trend + Buttons + icons | 49,028 | 273,204 | +19,928 | +21,792 |

### Isolating First-Instance Widget Costs

The single-widget samples give the cost of introducing a widget class for the first time (class code + one instance). Additional instances of the same class are cheaper — they only add the object's `sizeof`, not the class code.

| Widget Class | First Instance RAM | First Instance Flash | Notes |
|--------------|-------------------|---------------------|-------|
| Button | 208 | 4,416 | Includes Text class code |
| Display | 152 | 1,408 | Includes Text class code |
| Terminal | 268 | 2,096 | |
| Numkey | 308 | 7,584 | Inherits Button; includes internal grid rendering |
| IconButton | ~88 * | 4,960 | * Excluding icon bitmaps (see RODATA section) |
| Dial | ~200 † | ~3,968 † | † Estimated: sample includes 2 Buttons |
| Slider | ~100 ‡ | ~680 ‡ | ‡ Estimated from multi-widget sample |

\* IconButton sample RODATA jumps by +7,328 bytes due to two 60×30 icon bitmaps (1,800 pixels × 2 bytes × 2 icons = 7,200 bytes + padding).

† Dial sample includes 2 Buttons. Subtracting Button first-instance cost (208 RAM, 4,416 flash) gives ~200 RAM and ~3,968 flash for Dial class + instance.

‡ Slider sample includes 3 Sliders + 4 Displays. After accounting for Display class overhead and additional instances, approximate per-class cost.

---

## Runtime Dynamic Allocations

These `malloc`/`new` calls happen in constructors and `init()` methods. They are **not** visible in compile-time static RAM — they come from the heap at runtime.

### Framework Core

| Component | Allocation | Size | When |
|-----------|-----------|------|------|
| Canvas constructor | `new Guitft(cs, ds)` | ~200 bytes (Adafruit_ILI9341 object) | Constructor |
| Canvas::widgets | `StackArray<Widget*>` | 8 bytes initial (2 pointers), grows by 2× | Constructor, push() |
| TouchDriver_XPT2046 | `new XPT2046_Touchscreen(cs)` | ~40 bytes | Constructor |
| Screen::widgets | `StackArray<Widget*>` | 8 bytes initial, grows by 2× | Constructor |

### Per-Widget Allocations

| Widget | Allocation | Size Formula | Typical Size |
|--------|-----------|-------------|--------------|
| **Indicator** (base class) | `malloc(BUF_SIZE+1)` for `buf` | 9 bytes | 9 bytes |
| **Dial** | `malloc(BUF_SIZE+1)` for `buf` | 9 bytes (+9 leaked†) | 18 bytes |
| **Gauge** | `malloc(BUF_SIZE+1)` for `buf` | 9 bytes (+9 leaked†) | 18 bytes |
| **Trend** | `malloc(BUF_SIZE+1)` + `malloc(maxValues)` | 9 + maxValues bytes (+9 leaked†) | 34 bytes (ESP8266, maxValues=16) |
| **Slider** | (inherits Indicator's buf) | 0 additional | 9 bytes (from Indicator) |
| **Button** | `new Text(textLength)` | ~8 + textLength+1 bytes | ~17 bytes (textLength=8) |
| **Display** | `new Text(textLength)` | ~8 + textLength+1 bytes | ~17 bytes (textLength=8) |
| **Terminal** | `malloc((maxCharacters+1))` × lines | lines × (maxCharacters+1) bytes | ~110 bytes ‡ |
| **Numkey** | `malloc(DISPLAY_SIZE+1)` + inherits Button's Text | 7 + ~17 bytes | ~24 bytes |
| **Popup** | `malloc(text_length+1)` + 2× `malloc(9)` + inherits Button's Text | text_length+1 + 18 + ~17 | ~44 bytes |
| **Buttongrid** | `malloc(7)` + `malloc(qty×8)` + `malloc(qty)` + inherits Button's Text | 7 + qty×9 + ~17 bytes | ~60 bytes (qty=4) |
| **OptionButton** | `malloc(DISPLAY_SIZE+1)` + inherits Button's Text | 7 + ~17 bytes | ~24 bytes |
| **IconButton** | `malloc(labelSize+1)` + inherits Button's Text | labelSize+1 + ~17 bytes | ~24 bytes |

† **Memory leak note**: Dial, Gauge, and Trend constructors allocate `buf` via `malloc(BUF_SIZE+1)`, but the Indicator base class constructor already allocated `buf`. The derived class overwrites the pointer, leaking the base class's 9-byte allocation. This is 9 bytes per Indicator subclass instance.

‡ **Terminal example**: `Terminal(240, 50, TERMINAL_SCROLL_UP, 1)` at fontSize=1 yields approximately 3 lines × 37 characters = ~114 bytes for line buffers. The exact count depends on the widget's width, fontSize, borderWidth, and bleed values.

### StackArray Growth

`StackArray` starts at 2 elements (8 bytes for `Widget*`) and doubles when full. A canvas with 8 widgets will trigger 2 resizes: 2→4→8, allocating 32 bytes (previous allocations are freed via `realloc`). A Screen with 5 widgets: 2→4→8, same 32 bytes.

---

## RODATA: Icon Bitmaps

Icon bitmaps are the largest single contributor to RAM consumption. On ESP8266, `const uint16_t[]` arrays are placed in RODATA which is loaded into RAM (not flash-cached like IROM code).

| Icon Size (pixels) | Storage | RAM Cost |
|--------------------|---------|----------|
| 30×30 (900 px) | `const uint16_t[900]` | 1,800 bytes |
| 50×50 (2,500 px) | `const uint16_t[2500]` | 5,000 bytes |
| 60×30 (1,800 px) | `const uint16_t[1800]` | 3,600 bytes |

**Each icon bitmap costs width × height × 2 bytes of RAM.**

To reduce icon RAM on ESP8266, store bitmaps in flash using `PROGMEM`:
```cpp
const uint16_t icon_data[] PROGMEM = { ... };
```
This moves the data from RODATA (RAM) to flash, freeing heap space. Note that `PROGMEM` data must be read with `pgm_read_word()` — the `drawRGBBitmap` overload that takes `const uint16_t*` from PROGMEM handles this automatically in Adafruit_GFX.

---

## Estimation Guidelines

### Step 1: Start with the Framework Base

| Component | RAM | Flash |
|-----------|-----|-------|
| ESP8266 system + Serial | 28,072 | 235,588 |
| Guitft (TFT driver) | +656 | +12,268 |
| Canvas_Touch + touch | +372 | +3,556 |
| **Framework base total** | **29,100** | **251,412** |

### Step 2: Add Widget Class Costs (First Instance)

For each **unique widget class** used in your sketch, add the first-instance cost:

| Widget Class | RAM | Flash |
|--------------|-----|-------|
| Button | +208 | +4,416 |
| Display | +152 | +1,408 |
| Dial | +200 | +4,000 |
| Gauge | ~200 | ~3,500 |
| Slider | +100 | +700 |
| Trend | +180 | +3,000 |
| Terminal | +268 | +2,096 |
| Numkey | +308 | +7,584 |
| Popup | ~250 | ~3,000 |
| Buttongrid | ~250 | ~4,000 |
| IconButton | +88 | +4,960 |
| OptionButton | ~100 | ~1,000 |
| Screen | ~50 | ~500 |

### Step 3: Add Per-Instance Costs for Additional Widgets

After the first instance of a class, each additional instance adds roughly the object `sizeof`:

| Widget Class | Per Additional Instance (RAM) |
|--------------|------------------------------|
| Button | ~80 bytes |
| Display | ~60 bytes |
| Dial | ~120 bytes |
| Gauge | ~100 bytes |
| Slider | ~110 bytes |
| Trend | ~120 bytes |
| Terminal | ~140 bytes |
| Screen | ~60 bytes |

### Step 4: Add Runtime Allocations

Sum up the dynamic allocations from the table above for each widget instance.

### Step 5: Add Icon Bitmaps (if using IconButton)

Each icon: **width × height × 2 bytes** of RAM (unless using `PROGMEM`).

### Example Estimate: Typical Application

A sketch with 1 Canvas_Touch, 2 Screens, 4 Buttons, 2 Displays, 1 Dial, 1 Slider:

| Component | RAM | Flash |
|-----------|-----|-------|
| Framework base | 29,100 | 251,412 |
| Button class (1st) | +208 | +4,416 |
| 3 more Buttons | +240 | — |
| Display class (1st) | +152 | +1,408 |
| 1 more Display | +60 | — |
| Dial class (1st) | +200 | +4,000 |
| Slider class (1st) | +100 | +700 |
| Screen class (1st) | +50 | +500 |
| 1 more Screen | +60 | — |
| Runtime mallocs (~10 widgets) | +250 | — |
| **Estimated total** | **~30,420** | **~262,436** |
| **Remaining heap** | **~51,500** | |
| **Usage** | **37%** | |

### Quick Rules of Thumb

1. **Framework base**: ~29 KB RAM, ~251 KB flash (fixed cost)
2. **Per widget instance**: ~60–270 bytes RAM (static) + ~17–114 bytes RAM (runtime malloc)
3. **Icon bitmaps**: width × height × 2 bytes RAM each (dominant cost if used)
4. **Available heap on ESP8266**: 81,920 − static RAM − ~3 KB system overhead
5. **Comfortable headroom**: Keep static RAM + estimated runtime allocations under ~65 KB to leave ~15 KB for stack and runtime overhead
6. **Warning zone**: Above 70 KB total RAM usage risks stack overflow and heap fragmentation

### ESP8266 vs ESP32

| Parameter | ESP8266 | ESP32 |
|-----------|---------|-------|
| Total RAM | 81,920 bytes | 327,680 bytes (DRAM) |
| MAX_TREND_VALUES default | 16 | 32 |
| SPI fast path | Yes (writeBytes) | Yes (writeBytes) |
| Practical widget limit | ~15–20 widgets | ~50+ widgets |
| Icon budget (no PROGMEM) | 2–4 small icons | 20+ icons |

---

## Measured vs Estimated: Benchmark Validation

The full benchmark sketch (Dial + Trend + 3 Sliders + 4 Displays + Buttons + icon bitmaps + benchmark variables):

| Metric | Measured | Notes |
|--------|----------|-------|
| Static RAM | 49,028 bytes | Includes ~18 KB of icon RODATA |
| IROM (Flash) | 273,204 bytes | All widget + benchmark code |
| Remaining for heap | ~32,892 bytes | Before runtime mallocs |
| Runtime mallocs (est.) | ~400 bytes | All widget buffers + StackArrays |
| **Effective free heap** | **~32,500 bytes** | Comfortable for this configuration |

Without icon bitmaps, this same sketch would use ~31 KB static RAM — well within comfortable range.
