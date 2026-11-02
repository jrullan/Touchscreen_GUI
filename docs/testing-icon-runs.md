# IconButton same-color bitmap runs

## Contract

`IconButton::drawRGBABitmap` reads RGB565 words with `pgm_read_word` and emits
maximal horizontal **same-color** runs. An opaque span containing different
colors is not a single uniform-color transfer. When `useTransparency` is true,
only words equal to the supplied transparent color are skipped; when false,
that color is painted too. No RGB565 conversion is performed.

The destination rectangle is intersected with the rotated display dimensions
before any bitmap read. Coordinates, source row multiplication, and source
indices use 32-bit intermediates, including on targets with 16-bit `int`.
Coordinates are narrowed only after clipping. NULL, nonpositive dimensions,
hidden/unattached controls, and entirely offscreen rectangles do not read the
bitmap or open a transaction. The caller still supplies enough accessible
bitmap words for its declared dimensions; this API does not discover capacity
or extend AVR near-PROGMEM addressing to far flash.

A single stack `Guitft::ScopedWrite` covers the clipped traversal. A visible,
fully transparent image still opens one balanced transaction: this deliberately
avoids a preliminary flash scan or retained image state. Each visible source
word is read exactly once; clipped-out words are not read.

A compile-time overload prefers `writeFastHLine` on modern GFX/SPITFT panels.
Older drivers exposing only `drawFastHLine` use that uniform-color fallback,
without changing shared driver stubs or the public API. This also preserves the
legacy STM32 AS interface. A backend that implements fast lines using per-pixel
fallbacks remains correct but may not reduce hardware operations. The tests
exercise both overloads: modern real-GFX panel seam and old host driver seam.
The actual legacy STM32 board/toolchain is not exercised here.

There is no framebuffer, heap allocation, pixel buffer, image cache, new object
state, or redraw suppression in production. Auxiliary storage is constant-size
stack scalars plus the transaction guard; target stack/code-size deltas require
a target build. Host `sizeof(IconButton)` stays **128**, `sizeof(Button)` **104**
in both baseline and optimized runs. Existing icon flash assets remain **27,200
bytes**, verified with host and installed Xtensa assembly section tests.
Callbacks, debounce, pressed/unpressed selection, and unconditional explicit
`update()` behavior are unchanged, including mutable-image repainting.

## Tests

Run from the repository root:

```sh
python3 tests/host/run.py
python3 tests/rasterized/run_icon_runs.py --baseline
python3 tests/rasterized/run_icon_runs.py
python3 tests/rasterized/run.py
python3 tests/rasterized/run_widgets.py
python3 tests/rasterized/run_keyboard.py
python3 tests/rasterized/run_xaxis.py
python3 tests/transactions/run.py
python3 tests/host/check_returns.py
python3 -m unittest discover -s tests/tools -v
```

The new runner compiles actual production translation units and installed
`Adafruit_GFX.cpp`, under ASan/UBSan with leak checks. Set `ADAFRUIT_GFX_DIR` if
GFX is not installed under `~/Arduino/libraries/Adafruit_GFX_Library`. Missing
sources report SETUP REQUIRED, not success. Only the hardware and flash-read
boundary is instrumented; its stubs are isolated under `icon_stubs/`.

The panel seam models uniform-color transfers, transaction nesting, and physical
rotation mapping. It is not an SPI timing simulator. A widened independent
per-pixel reference is compared against the complete raster. It also predicts
maximal run counts without calling the production renderer. Tests cover all six
actual `icons.h` symbols, all four rotations, transparency on/off and a nonblack
key, seeded random RGB565, uniform/alternating colors, transparent holes, narrow
and remainder-width rows, partially clipped corners, fully outside rectangles,
NULL/negative/zero dimensions, extreme int16 coordinates, and source offsets
beyond signed 16-bit. Optimized run: **11,358 cases**. Baseline: **48 safe
in-bounds asset cases** using the actual `58eabc4:IconButton.cpp` retrieved with
`git show`, not a synthetic baseline implementation.

### Observed strict RED → GREEN

1. New host same-color batching assertion failed against original production:
   `same-color horizontal runs=0 expected=5`. After run batching, it passed with
   `same-color horizontal runs=5 expected=5`.
2. Before clipping was added, the real-GFX test failed with
   `clipped reads=14 expected=4` for a 7x2 image at (-3,-1). After clipping it
   passed with four reads, one uniform-color line, one balanced transaction.
3. A new modern-backend assertion failed because the renderer still called the
   `drawFastHLine` transaction wrapper. After compile-time direct-write selection,
   it passed with zero wrapper calls and direct `writeFastHLine` transfers.

All commands above passed after the final production change. Clean ESP8266
compatibility builds and the repository demo build also passed. The repository
`Touchscreen_GUI_Demo` was uploaded to the ESP8266/ILI9341/XPT2046 device with
firmware hash verification; the user confirmed that it worked. A private full-flash
backup of the previous firmware was retained before upload. This is physical
functional acceptance, not a hardware latency measurement; no measured speedup
is claimed.

## Reproducible operation counts

Each cell is **actual baseline per-pixel writes → optimized horizontal-line
transfers** for one in-bounds draw. Both versions emit identical pixels, read
each full image once, and open one balanced outer transaction. The optimized
version emits zero per-pixel writes at the instrumented panel boundary.

| Actual flash asset | Transparency off | Transparency on (black key) |
|---|---:|---:|
| slider_on (60x30) | 1800 → 315 | 1644 → 279 |
| slider_off (60x30) | 1800 → 297 | 1644 → 261 |
| on_full (50x50) | 2500 → 966 | 2476 → 954 |
| off_full (50x50) | 2500 → 763 | 2476 → 751 |
| lightbulb_on (50x50) | 2500 → 266 | 1192 → 176 |
| lightbulb_off (50x50) | 2500 → 310 | 1192 → 220 |

These are host operation counts, **not latency, physical SPI counts, or measured
speedup claims**. Real bus improvements depend on the backend and hardware.
Alternating colors correctly yield one run per drawn pixel; transparency alone
is not evidence that neighboring opaque colors can be merged.

Installed real-GFX source used for the recorded runs:
`Adafruit_GFX.cpp` SHA-256
`5047f01e8e6dd3029c73cb9447a915fc06675d2e1cc9ebf6a6c1126f556fcecd`.
