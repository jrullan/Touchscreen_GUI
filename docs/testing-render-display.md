# Display render-state regression tests

Run actual library implementations against the existing host hardware boundary:

```sh
python3 tests/host/run.py render_display
python3 tests/host/run.py
```

The runner enables AddressSanitizer, UndefinedBehaviorSanitizer and leak detection.
This fast runner's text boundary records operations but does not rasterize glyphs.
It requires no board access or external graphics source. For actual glyph cleanup
coverage, also run the separate rasterized suite described below.

## Rendering contract

- `setText(value, true)` retains an owned, capacity-truncated copy and requests an
  immediate **render-if-changed**. Equality is against the last successful render,
  not the previous input. Different suffixes outside capacity do not redraw.
- `setText(value, false)` changes only retained content, without display traffic.
  `update()` commits the current content/style. Deferred edits never overwrite
  the last-rendered snapshot.
- `update()` remains an unconditional repaint, preserving its existing use as an
  external-damage repair operation. `forceUpdate()` is an explicit alias.
  `show()` also unconditionally repaints, including the border. Thus callers
  previously using same-value `setText(value, true)` solely to repair external
  damage must instead use `update()`, `forceUpdate()`, or `show()`.
- The signature compares retained bytes and length, canvas and driver identity,
  x/y/width/height, font size, background/foreground/border colors and border width.
  Public field/content changes are detected without requiring setters.
- Hidden/unattached calls do not render or commit a snapshot. `show()` restores
  visibility and repaints. Detaching/reattaching to an identical target without
  rendering is not a notification of external damage; use explicit repaint.
- Ordinary text changes fill the current interior and draw the new centered
  value. This removes old tails, including the empty-text case. Old text bounds
  extending outside the interior are separately cleared using the committed
  backend-measured bounding rectangle, and any crossed border is redrawn.
  Geometry changes clear
  the previous widget rectangle to the canvas background before repainting the
  new location. Style/border changes repaint the entire current rectangle.

Display preserves the existing Widget centering origin and the backend's actual
text semantics: automatic wrapping remains enabled by default, and font size zero
still uses Widget's zero-size centering while Adafruit GFX rasterizes at size one.
After drawing, Display asks `getTextBounds` for the footprint using that exact
origin and the unchanged driver font/wrap state. The saved rectangle survives
later public font/geometry and driver wrap changes; cleanup does not recompute it.
No persistent or temporary wrap-state change is made. Older drivers without the
`getTextBounds` API retain a fixed-cell estimate (effective size at least one);
wrapped/custom-font cleanup guarantees require the GFX bounds API. Keep geometry
and text footprints within the target driver's coordinate range. This is not a
scene compositor: clearing old geometry restores the canvas background, not
widgets/images underneath; applications overlapping widgets must repaint those
layers. Switching canvas/driver does not dereference or erase the old target.
Text and its configured capacity remain owned by Display: do not rebind/delete
`contents`, replace its allocation, or enlarge `_textSize` behind the owner.

## Snapshot and memory cost

One `malloc(capacity + 1)` per Display with available Text storage holds the exact
last-rendered bytes. It never reallocates on updates. This preserves the original
Text allocation and owned-copy capacity; it does not use a collision-prone hash.
Snapshot allocation failure leaves Text intact and disables only suppression;
updates continue repainting safely. Missing Text object/storage is also safe.
The snapshot is freed in the Display destructor.

Inline additions are one snapshot pointer, a RenderState (two target pointers,
eight ints, two unsigned chars and four 16-bit bounds fields), and one validity
bool, plus ABI padding.
Exact total added object storage is `sizeof(Display) - sizeof(OriginalDisplayLayout)`
(the test's layout-only baseline mirrors the original members); heap payload is
exactly `capacity + 1` bytes, excluding allocator metadata. On the tested x86-64
host: original Display 72 bytes, new Display 152 bytes, **80 added inline bytes**.
At default capacity 8 that is **89 added payload bytes**, excluding allocator
metadata. MCU padding is ABI-dependent; the host size is not an MCU measurement.

## Evidence and operation counts

Tests were written and run against the original implementation before changes:
capacity-truncated identical input failed the zero-operation assertion. Later
RED runs exposed missing changed-border rendering, missing explicit repaint API,
and border damage caused by overflowing old text cleanup.

The workload initializes a borderless 120x40 Display at (20,30), capacity 8,
font 2, with `1234`, clears recorded operations, then submits 100 immediate updates.
The changed workload alternates `5678` and `1234`; all submissions change content.

| Workload | Before | After |
| --- | ---: | ---: |
| 100 same retained values | 200 operations | 0 operations |
| 100 changed values | 200 operations | 200 operations |

Before: each update draws old text in background color, then new text. After:
changed values fill the interior then draw text; repeated values emit nothing.
These are actual host draw-operation records, not SPI timings or byte counts.
Geometry/style/overflow/forced repaint paths may intentionally perform more work.

Coverage includes truncation, capacity 0/255, aliases, direct content mutation,
public geometry/colors/font/border mutations, target changes, hidden/unattached
state, deferred commits, centered shorter and empty values, old geometry and
font footprint cleanup, crossed-border restoration, external damage repair,
Text allocation failure and snapshot allocation failure. The stub records text
content, centering, font dimensions and color but does not rasterize glyphs;
stale-cleanup checks place sentinel pixels at the old footprint and verify that
real fill operations clear them. No glyph-rasterization claim is made.

The final full discovered host suite (11 suites) passed with ASan/UBSan/leak
detection, including existing text, visibility, lifecycle, allocation-failure and
chart tests; `python3 tests/host/check_returns.py` also passed.

## Real Adafruit GFX rasterized regressions

```sh
python3 tests/rasterized/run.py
# Optional explicit source location (installed Arduino library is the default):
ADAFRUIT_GFX_DIR="$HOME/Arduino/libraries/Adafruit_GFX_Library" python3 tests/rasterized/run.py
```

This independent ASan/UBSan/leak runner compiles the actual installed
`Adafruit_GFX.cpp`, including its actual `glcdfont.c` font data, alongside the
library sources (verified here with Adafruit GFX Library 1.13.0). It prints
source paths and SHA-256 hashes for provenance. No
font data or rasterizer is fabricated or copied into this repository. Only the
Arduino/Print and bus headers and ILI9341 panel boundary are substituted under
`tests/rasterized/stubs`; the panel implements `drawPixel` into memory. All GFX
text layout, glyph decoding and primitive rasterization are the installed source.
The runner never adds `tests/host/stubs` to its include paths. These isolated
boundary files derive from the independent review's `display-review-real-gfx`
scratch probe, not from a new glyph implementation.

Before the fix, `python3 tests/rasterized/run.py` exited 1 with:

```text
wrap: rendered=272 stale=68 witness(7,62)=65535
font0: rendered=68 stale=52 witness(82,62)=65535
```

After the fix, both cases report `stale=0`, a black witness pixel (`0`), and PASS.
The exact cases use a portrait 240x320 target, Display capacity 8, no border,
black background, white foreground and `8888` followed by empty:

- wrap: (210,30), 30x20, font 2;
- font0: (50,50), 20x10, font 0.

Assertions require actual pre-clear glyph counts and the named white witness
pixels, then count **all** remaining white pixels, so an inert print stub cannot
pass. Each case also compares shorter-text pixels against a clean render and
checks cleanup after font/wrap-state changes and preservation of disabled wrap.

CI must install/provide the real Adafruit GFX Library source and invoke this
runner separately; the fast suite does not discover it. With missing source,
the runner exits nonzero with `SETUP REQUIRED` and explicitly states that no
raster test ran. Missing setup is not a PASS or a simulated raster result.
No hardware SPI timing, panel output, or non-GFX legacy-backend rasterization
claim is made.
