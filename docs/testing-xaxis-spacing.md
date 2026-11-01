# Adaptive X-axis spacing (Trend / MultiTrend)

## Contract and compatibility

`TrendBase` owns the implementation used by both charts. The previous non-STM32
`majorStep = 2` selected every other sample regardless of its pixel spacing;
255 samples in a 120px widget produced overlapping `254` and `252` labels at
`[34,52)` and `[35,53)`. The previous STM32 width-derived sample step also did
not measure label widths. Both paths now use the same pixel-fit rule.

- Preserve even-offset candidates plus both window endpoints. Do not invent new
  values or change sample history, window mapping, or index-to-X coordinates.
- Index labels remain the absolute sample index. Positive `maxX` remains seconds
  over the *selected window*, with offset-based integer interpolation; labels
  below 60 are decimal seconds and labels from 60 upward are `m:ss`. Nonpositive
  `maxX` remains index mode; a single-sample time window remains `0`.
- Measure each fully formatted candidate as `strlen(buf) * FONT_X`, in `int`.
  Render through a stack-local `Adafruit_GFX` pixel forwarder with its own
  default classic font, using explicit size-one `drawChar` calls. Unlike
  `Guitft::drawString`, this cannot inherit the TFT's proportional `gfxFont`.
  Public widget sizes 0, 1, 2, 31, and 255 all retain 6x8 classic cells. The
  caller's font, cursor, wrap, text size and foreground/background colors are
  untouched. No display-wide font reset, temporary state restoration, public
  Guitft API, heap framebuffer or Y-scale/drawNumber change is involved.
- Center labels on unchanged sample coordinates, clamping both horizontal edges
  into the widget. Reject a complete label wider than the widget; never truncate
  text or let a signed negative origin narrow into a huge unsigned coordinate.
- Greedily admit candidates right-to-left with at least **2px** between complete
  text-cell rectangles. Reserve the oldest endpoint when its bounded rectangle
  and the newest endpoint can coexist. Otherwise do not force overlapping
  endpoints (newest takes priority when both cannot coexist).
- Existing label contents and sparse candidate sets remain unchanged wherever
  they fit. Major ticks follow admitted labels, with endpoint ticks retained;
  remaining minor ticks retain the existing minor-step rule.
- Keep scale-strip erase, ticks and labels inside allocated widget bounds.
  The old label baseline extended below the widget with the default reserved
  strip; move it upward only as necessary. Reject negative strip height,
  suppress text without vertical room, and bound tick lengths. These guarantees
  concern X-axis operations, not unrelated Y chrome or optional outlying markers.
- No public signatures, public data layout, sample limits, render-cache fields,
  cache-allocation paths, or incremental commit logic change. A protected
  formatter/measurement helper adds no object state and no heap allocation.
  Unchanged commits still produce zero operations.

`maxX` is an `int` duration in **seconds**, not `millis()` or a timestamp. No
clock acquisition or rollover arithmetic belongs in this API. Interpolation
widens the multiplication to `int64_t` before dividing. Tests include `INT_MAX`
seconds (`35791394:07` on this 32-bit-int host), as well as 59/60/61,
599/600/601 and 3599/3600 digit/format transitions. No millisecond API was added.

## Strict TDD evidence

Starting revision: `cd9294b`.

1. Added only the host reproducer before production changes.
2. `python3 tests/host/run.py xaxis_spacing` exited 1 (SIGABRT): the original
   label exceeded the allocated vertical chart bounds.
3. Reordered the independent assertions to report spacing first, without
   changing production. The same command exited 1 (SIGABRT), reporting:

   ```text
   overlap: text:254 [34,52) vs text:252 [35,53)
   Assertion `false && "X labels require a two-pixel gap"' failed.
   ```

4. Implemented the measured admission and bounded-axis rendering. The same
   target exited 0, `xaxis_spacing: PASS`.
5. Expanded regression coverage without additional production changes. The
   target exited 0 with **11,880 matrix cases per chart** and `PASS xaxis_spacing`.
6. `python3 tests/host/run.py` exited 0: all **23** discovered host targets passed
   under ASan, UBSan and leak detection. The chart benchmark reported
   `changed20=22160 unchanged20=0 sizeofBase=376 sizeofTrend=392 sizeofMulti=408`.

## Coverage / reproduction

```sh
python3 tests/host/run.py xaxis_spacing
python3 tests/host/run.py
```

The runner discovers `test_xaxis_spacing.cpp` automatically; no runner or CI
changes are needed. It compiles real chart sources with host display boundaries.
The host boundary records both print submissions and forwarded classic glyphs
as text-cell rectangles (not real glyph bitmaps). Existing assertions remain
unchanged; successive glyph cells coalesce only when adjacent and same-style,
and the destination is identified by actual virtual drawPixel forwarding. It:

- Covers 12 widths (0 through 230), five public font sizes, Y-scale on/off,
  eleven time/index configurations and nine windows through index 254.
- Requires 2px nonoverlap, complete label bounds, valid candidate text and both
  endpoints whenever their clamped rectangles fit together.
- Checks **every X-axis operation** lies inside allocated widget bounds.
- Checks unchanged mapping, original sparse labels `2,4,5`, unchanged history,
  rejection of an 18px label in 12px, and one-label-only behavior at 6px.
- Covers small heights, X-scale disabled (no axis operations / no X labels),
  both chart classes, zero wrapped C allocations during axis draws, and zero
  drawing or allocation for ten unchanged commits.

## Independently confirmed custom-font blocker: retained raster regression

`python3 tests/rasterized/run_xaxis.py` builds the actual installed
`Adafruit_GFX.cpp` and `glcdfont.c` with actual chart sources, replacing only
Arduino/bus/panel boundaries. It reports source hashes and fails with SETUP
REQUIRED rather than passing if GFX or either FreeSans font is missing.

Strict RED, before changing production: both Trend and MultiTrend aborted with
exit 1, independently launched by the runner:

```text
RUN Trend
X-axis font leak: outside_axis_pixels=854 rightmost=162 widget_right_exclusive=130
Assertion `false && "X-axis must rasterize classic glyphs regardless of caller font"' failed.
RUN MultiTrend
X-axis font leak: outside_axis_pixels=854 rightmost=162 widget_right_exclusive=130
Assertion `false && "X-axis must rasterize classic glyphs regardless of caller font"' failed.
```

The reproducer uses 120x100, x=10/y=80, 255 samples, window 0..254, no Y scale,
3600 seconds and FreeSans18pt. The count above bounds the **X-axis strip**,
not merely the complete widget; it includes custom-font ink above that strip.
Only `drawXScale()` is called, so legacy Y labels, sample lines and markers
cannot contaminate this assertion.

GREEN after the local forwarder: exit 0, each chart reports
`real-GFX X-axis cases=361 PASS` and `xaxis real glyphs/state: PASS`.
The retained matrix compares every actual custom-font axis pixel against the
classic axis, requires real glyph ink and bounds all ink to the allocated
axis strip. It covers classic/FreeSans18pt/FreeSans24pt, wrap off/on, caller
sizes 1/3, widths 1/6/12/40/120, dense index/60s/3600s labels and disabled axes.
A subsequent real-GFX text render is compared against a separately configured
panel to verify retained font, cursor, wrapping, size and both text colors.
The unchanged host matrix still checks full-cell two-pixel spacing and endpoint
presence in 11,880 cases per chart. The intermediate host failure (labels
missing because its GFX drawChar was a no-op) was repaired at the stub boundary,
not by dropping or relaxing those coordinate/content assertions.

Verification commands, all exit 0 under ASan/UBSan/leak detection:

```sh
python3 tests/rasterized/run_xaxis.py
python3 tests/host/run.py xaxis_spacing
python3 tests/host/run.py
python3 tests/rasterized/run.py
python3 tests/rasterized/run_keyboard.py
python3 tests/rasterized/run_widgets.py
python3 tests/host/check_returns.py
python3 tests/tools/test_check_arduino.py
python3 tests/tools/test_icons.py
python3 tests/transactions/run.py
python3 tests/xpt2046/run.py
```

All 23 host targets passed. Existing raster suites passed Display wrap/font0,
six keyboard cases, bars and Terminal. Unchanged chart benchmark remains
`changed20=22160 unchanged20=0 sizeofBase=376 sizeofTrend=392 sizeofMulti=408`.
Strict return/const/sentinel checks, clean-build command tests, icon tool tests,
real-GFX transaction dispatch and bundled-XPT2046 host regressions also passed.

### STM32 backend API compatibility (host syntax check only)

Before using backend APIs, a scratch forwarder probe was compiled with
`__STM32F1__`, the real upstream `Adafruit_GFX_AS.h`, `Load_fonts.h` and
`Adafruit_ILI9341_STM.h` from
`rogerclarkmelbourne/Arduino_STM32/STM32F1/libraries/` (GitHub master), installed
Adafruit_GFX and a minimal SPI declaration seam. It exited 0. After the fix,
the actual `TrendBase.cpp` also passed `g++ -std=c++17 -fsyntax-only
-DARDUINO=10819 -D__STM32F1__`, with those headers before the raster Arduino
stubs and installed GFX include paths. The forwarder requires only the common
GFX constructor, drawPixel and six-argument drawChar APIs; it does not require
modern writeFillRect overrides or STM32 font-state getters/setters.

This is declaration/API compatibility evidence, not an STM32 board build or
DMA/SPI/runtime claim. No Arduino compilation, flashing, hardware access,
commit, push, Designer changes or CI edits were performed for this blocker.
