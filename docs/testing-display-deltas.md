# Display character-difference rendering

## Behavior

`setText(text, true)` now repaints only changed character cells when a conservative fixed-cell fast path is proven safe. It compares against the existing last-rendered text snapshot, not the most recent deferred setter. No second text cache, framebuffer or update-time allocation is added.

Eligibility requires an introspectable classic GFX font, nonzero fontSize, unchanged font/wrap/CP437/rotation/display geometry, identical string length, unchanged widget geometry/style, and a complete single-line cell rectangle contained in both the widget interior and the panel. Control characters are rejected. Unknown/legacy backends and STM32 AS use the existing full-render fallback.

Changed cells use opaque foreground/background `drawChar`, including the spacing column. RGB565-equivalent foreground/background uses a solid cell because GFX treats equal colors as transparent. Unchanged cells and the border are not touched. Cursor, text size and transparent foreground state match the normal complete-string draw; shared font, wrapping and encoding are not changed.

A small read-only `Guitft::TextLayout` snapshot identifies shared font and layout state, including rotation and dimensions. A font/wrap/encoding/rotation change invalidates unchanged-string suppression instead of silently skipping rendering.

## Compatibility

- Existing constructors and setters are unchanged.
- `setText(text, false)` remains deferred.
- `update()`, `show()` and `forceUpdate()` remain unconditional repair paths.
- Different lengths, custom fonts, fontSize zero, clipping/overflow, style/position changes and missing snapshots retain full-render cleanup.
- Mutable direct content edits and aliased setter inputs are supported.
- Clean-rebuild library and applications because Display's private state/layout changes.

For a clock:

```cpp
clockDisplay.setText(timeText, true);
// Do not call update() immediately afterward: that deliberately requests repair.
```

An elapsed-time HH:MM:SS fixture is available in `tests/arduino/display_clock/`; it has no network or credentials and is not uploaded automatically.

## RED/GREEN and measurements

The new actual-GFX one-digit test first failed against production with `pixel writes=6684 budget=192`. After the delta path it passed with 192 writes, all confined to the changed cell. The runner also builds complete production headers/sources from base `f4a8f9c` using git archive: that actual baseline produces 6,684 writes in the same fixture, versus 192 now.

Host `sizeof(Display)` is 152 in the baseline and 168 now. Heap snapshot storage remains capacity+1 bytes; 1,000 changed updates make zero measured allocations. These are host object/pixel-write measurements, not physical SPI operations, hardware timing, MCU RAM usage or physical flicker acceptance.

## Verification

```sh
python3 tests/rasterized/run_display_deltas.py --baseline
python3 tests/rasterized/run_display_deltas.py --host
python3 tests/rasterized/run_display_deltas.py
python3 tests/host/run.py display_deltas render_display
python3 tests/rasterized/run.py
python3 tests/rasterized/run_widgets.py
```

Actual Adafruit GFX raster comparisons cover all four rotations, three scales, borders/no borders, seconds/minutes/hours rollover, unchanged strings and zero writes outside changed cells. Fallback comparisons cover custom fonts, wrapping, clipped/overflow text, controls, shortening, style/movement, deferred setters, hidden/show and explicit external-damage repair. Ownership tests cover missing Text, failed snapshot, first render, null/aliased input and direct mutations. Sanitizers and leak detection are enabled.

Integration passed the 31-executable host suite (split into bounded groups), then
the additional maintained `display_delta_overflow` regression and focused Display
reruns after review correction. The eligibility guard uses int64 arithmetic before
narrowing and rejects the review's INT_MAX-coordinate cases and large cell extents
safely; this does not claim to fix every pre-existing full-render arithmetic case.
Existing Display/widget raster tests and clean exact-library ESP8266 builds,
including the clock fixture, dashboard and repository demo, passed. CI invokes
both allocation/operation and real-GFX delta tests plus all 32 host executables.
No firmware upload or hardware timing has been performed for this change.
