# Selective Keyboard mode repaint

Shift and Symbol touches compare old/new mode cell outputs and repaint only changed key interiors. They do not redraw borders, the text field, Backspace, Space, Cancel, OK, or unchanged punctuation/command keys. `drawFrame()`, `update()` and `show()` remain unconditional full-repaint paths. Public APIs, literal typing, completion callbacks, self-deleting handlers, overlay restoration, classic-font isolation and opt-in `TextStorage::None` are unchanged.

No framebuffer cache, heap allocation or object member was added. Mode comparisons use the effective RGB565 background, suppress invisible foreground changes and use the same clipped-glyph visibility predicate as full rendering. Public direct geometry/style/mode edits still require explicit `update()` to paint those edits; touch deltas are not an external-damage recovery mechanism.

## Run

From the repository root:

```sh
python3 tests/host/run.py keyboard_modes keyboard optional_allocations
python3 tests/rasterized/run_keyboard_modes.py
python3 tests/rasterized/run_keyboard.py
```

All runners use ASan/UBSan with leak detection. The new raster runner compiles the real installed `Adafruit_GFX.cpp`, its included `glcdfont.c`, and actual production translation units. Set `ADAFRUIT_GFX_DIR` if not installed at `~/Arduino/libraries/Adafruit_GFX_Library`. Missing real source is a setup failure, never a fake-font fallback. It prints source hashes. The isolated `keyboard_modes_stubs/Adafruit_ILI9341.h` replaces only the hardware panel; it records every raster pixel write without altering GFX font behavior. Shared runners, shared stubs and CI are not modified by this change.

## Evidence

Vertical RED → GREEN slices were executed before each corresponding implementation:

1. Standard Shift: host assertion rejected full-frame/border writes; real-GFX pixel equality passed but its strict write-budget assertion failed. Replacing mode full redraw with cell-interior deltas made both pass.
2. Symbol-mode Shift with `highlightColor = bgColor + 65536`: both operation/no-write assertions failed, then passed after comparing effective 16-bit background colors.
3. Foreground equal to background: host expected only the Symbol background fill and real GFX required fewer than 1000 writes. Both failed before suppressing invisible character changes and passed afterward.
4. Left clipping (`x=-198`, width 231): host expected four changed interiors, failed on unnecessary fragment erases, then passed after sharing the full-draw glyph visibility predicate.

The broader real-raster regression executes **704 physically reachable transitions** across all four driver rotations, classic/inherited FreeSans18pt7b state, keyboard font sizes 1/2, border widths 0/2, uneven dimensions and left/right/bottom clipping. Each transition compares every pixel against a clean full redraw. Independently derived old/new raster differences identify changed cell interiors; every recorded write must lie inside one of those interiors. This also forbids writes to unchanged cells, text, controls and borders. Subsequent real rendering checks caller font/anisotropic size/colors/wrap/cursor preservation. Existing suites cover invalid size-zero layouts, callbacks deleting keyboards, literal input and overlay restoration.

### Operation counts

Host fill calls for default visible geometry:

| Mode action | Previous full redraw | Selective |
| --- | ---: | ---: |
| Shift in alphabet mode, either direction | 90 | 27 |
| Shift in symbols mode, either direction | 90 | 1 |
| Symbol toggle, all four starting modes | 90 | 39 |
| Shift in symbols, equal effective highlight/background | 90 | 0 |

Measured real-GFX pixel writes (240×192 keyboard, border 2, font 1, text `preserved`):

| Starting mode (`Shift + 2*Symbol`) | Toggle | Selective | Full redraw of result |
| --- | --- | ---: | ---: |
| 0 | Shift | 15520 | 82135 |
| 0 | Symbol | 22261 | 82073 |
| 1 | Shift | 15443 | 82058 |
| 1 | Symbol | 22261 | 82073 |
| 2 | Shift | 565 | 82073 |
| 2 | Symbol | 22246 | 82058 |
| 3 | Shift | 565 | 82073 |
| 3 | Symbol | 22323 | 82135 |

Host `sizeof(Keyboard)=192`, `sizeof(Button)=104`, fixed edit buffer 49 bytes; no data-member changes. Mode touches allocate nothing. The existing allocation suite reports default Keyboard 2 allocations/25 retained requested bytes, versus `None` 0 allocations/0 bytes, with zero retained after destruction for both styled/default constructions. These are host ABI/allocator measurements, not MCU sizes or physical display timings. No hardware, commits or pushes were used.
