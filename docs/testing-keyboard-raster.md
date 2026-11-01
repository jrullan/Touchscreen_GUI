# Keyboard: real-GFX pixel regressions

Run from the repository root:

```sh
python3 tests/rasterized/run_keyboard.py
# Select cases while investigating a failure:
python3 tests/rasterized/run_keyboard.py labels restore edges modes font0
python3 tests/rasterized/run_keyboard.py customfont
```

The runner builds the actual `Keyboard.cpp` and other root production translation
units, together with the installed `Adafruit_GFX.cpp` (including its real
`glcdfont.c`). Only Arduino, bus, touch-driver, and panel boundaries are replaced.
The include order follows `run_widgets.py`: transaction panel stub, rasterized
Arduino stubs, installed GFX, project root. The no-op GFX from `tests/host/stubs`
is deliberately absent. The existing transaction panel's RGB565 framebuffer is
the observed output, not a synthetic rendering of production method calls.

Requirements: `g++`, Python 3, and Adafruit GFX sources, including
`Fonts/FreeSans18pt7b.h`. The default GFX directory is
`~/Arduino/libraries/Adafruit_GFX_Library`; override it with
`ADAFRUIT_GFX_DIR`. Missing sources are a setup failure, never a test pass.
The runner reports source hashes, uses `$TMPDIR/gui-keyboard-raster` (or the
repository `.host-build` fallback), compiles with AddressSanitizer and
UndefinedBehaviorSanitizer, and enables leak detection and fail-fast sanitizer
options. Each selected case runs separately, so a regression does not suppress
results from unrelated cases. Any case failure makes the runner exit nonzero.

## Assertions

- **`labels`**: widths 220, 231, and 240 and a non-divisible height of 197;
  whole-frame equality against an independent real-GFX classic-font oracle for
  every key, command label, border, and retained buffer tail. Explicit foreground
  witnesses prevent blank command labels from passing. Shorter and empty text
  must remove all prior buffer glyphs. Pixels outside the overlay remain unchanged.
- **`restore`**: both Canvas-registered and Screen-registered keyboards; three
  show/hide cycles restore an exact pre-overlay framebuffer, including distinct
  canvas/screen backgrounds, a visible Screen Button, and a visible Canvas
  Button. A hidden Screen Button remains hidden and unpainted. Hidden keyboard
  update/hide calls perform no writes. The test deliberately avoids
  `Screen::show()` when constructing its expected baseline because that method
  re-shows all children.
- **`edges`**: clipping at right/bottom, negative origins, mostly off-screen
  placement, and bottom-aligned placement. Both caller wrap states are tested.
  Whole-frame equality detects wrapped or stray glyphs even *inside* the overlay
  (for example at the left edge). A subsequent real-GFX print probes that the
  caller's wrap state was preserved.
- **`modes`**: actual touch handling switches shift/special modes and enters
  `qQ#^<\\`, followed by dedicated backspace and Space actions. Whole-frame
  comparison checks labels, typed buffer, and active-mode highlighting. Literal
  `#`, `^`, `<`, and backslash must not become command actions.
- **`customfont`**: installs the actual FreeSans18pt7b through the public TFT API
  before showing the keyboard. No assumption is made about whether the keyboard
  should normalize to the classic font or safely accommodate the custom font.
  It must paint visible command labels, preserve pixels outside its rectangle,
  and keep key glyphs off cell borders; hide must restore the background.
- **`font0`**: preserves the current documented fully inert `fontSize=0`
  behavior; it does not demand normalization to size one.

The classic-font cases also verify balanced transactions at the real-GFX
virtual dispatch boundary. These are host pixel/lifecycle checks, not MCU flash,
physical SPI timing, touch calibration, or hardware validation. The stock panel
stub discards out-of-display drawPixel calls; framebuffer assertions prove
visible footprint containment, not that no discarded low-level call occurred.

## Observed regression: inherited custom font

The initial sanitizer run passed `labels`, `restore`, `edges`, `modes`, and
`font0`. `customfont` failed with:

```text
FAIL: FreeSans18pt7b inherited by keyboard: stray pixel outside keyboard (16,57), color=0xffff
```

Minimal production API reproduction:

```cpp
Keyboard keyboard;
keyboard.setSize(220, 192);
keyboard.borderWidth = 2;
keyboard.fontSize = 1;
keyboard.setColors(BLACK, WHITE, RED);
canvas.add(&keyboard, 10, 70, false);
canvas.tft->setFont(&FreeSans18pt7b);
canvas.tft->setTextWrap(true);
keyboard.setText("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
keyboard.show();
```

The keyboard rectangle starts at y=70, but a white glyph pixel appears at y=57.
`Keyboard::paintCell()` measures and positions labels using classic-font 6x8
advances; `Guitft::drawString()` retains the backend's custom font, whose glyphs
have different advances and baseline offsets. Its classic-font fit test
did not bound the actual glyphs. The regression assertion is retained without
weakening the bounds requirement. The correction uses a stack-local classic-font
GFX forwarding renderer, avoiding mutation of the shared TFT font, cursor, wrap,
size, or color. All six cases now pass, including the remaining custom-font
visibility, border, and hide assertions. Independent final review also reran the
full suite successfully. These results do not establish physical acceptance.
