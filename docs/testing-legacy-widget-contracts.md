# Legacy widget input and rendering contracts

These are deliberately retained legacy behaviors, not the newer `Keyboard`
accept/cancel session API. The production change covered here is limited to
non-maintained `Buttongrid` first-touch highlighting. `Numkey` and `Dial` are
characterized without production changes.

## Run the host checks

From the repository root:

```sh
python3 tests/host/run.py buttongrid_highlight legacy_input_contracts
python3 tests/host/run.py
```

The runner compiles the actual library translation units with the host Arduino
and display boundaries, AddressSanitizer, UndefinedBehaviorSanitizer, and leak
detection. No board, flashing, or Arduino build is involved. Display operations
and selected pixels are checked; the shared display stub does not rasterize
text glyphs or needle triangles. These tests do not establish physical touch
calibration, SPI timing, or hardware rendering fidelity.

## Buttongrid

- IDs are one-based, row-major. Touches must be strictly inside a cell, not on
  its border. Tests install the grid's typed callback and advance the host clock
  beyond the inherited debounce interval; an immediate second touch must not
  invoke the callback or paint.
- With `BUTTONGRID_HIGHLIGHT` and without `BUTTONGRID_MAINTAINED`, a valid press
  paints the current cell with `highlightColor` and redraws its label, even when
  `lastPressed == 0`. A valid previous cell is separately restored with
  `bgColor` and its label before painting the new cell.
- The previous implementation incorrectly placed both erase and current-cell
  painting inside the `lastPressed != 0` condition. The first-touch regression
  was run against that implementation and failed at the expected drawing
  operation-count assertion, before the production condition was changed.
- `clear()` redraws the grid and resets `lastPressed` to zero. The next touch
  must still highlight. It does **not** reset the maintained `states` array;
  clearing that state is not added by this fix.
- `BUTTONGRID_MAINTAINED` independently toggles each cell's state. With
  highlighting, toggling on paints `highlightColor`, and toggling off paints
  `bgColor`, followed by its label. Without highlighting, callbacks and state
  transitions still occur but a press draws nothing. Non-maintained presses do
  not toggle the maintained states array.
- `checkTouch()` retains its legacy `true` return (allow further checking).
  Callback ordering, debounce arithmetic, and null-handler behavior are not
  redesigned by the paint fix.

Coverage: `tests/host/test_buttongrid_highlight.cpp` checks the first touch,
second touch/previous-cell restoration, an immediate debounced touch, the first
press after `clear()`, maintained on/off/independent toggles, and both maintained
and non-maintained no-highlight configurations.

## Numkey: explicit target bridge, not automatic acceptance

`show(Button*)` stores a borrowed target pointer and shows the keypad. It does
not copy the target's text into the keypad, does not clear existing keypad text,
and does not automatically wire the target's callback. Keep the target alive
while the keypad retains this pointer.

Pressing `=` sets `entry` from `getNum()` and invokes the keypad's own registered
`Numkey*` callback. The existing optional `autoremove` path may pop the keypad
if it is the top Canvas widget. Neither the target's text nor its callback is
automatically updated. The characterization test sets `autoremove=false` to
isolate the callback/target contract; it does not claim overlay lifecycle or
callback-driven deletion safety.

`targetEventHandler()` explicitly calls the stored target's `Button*` callback
when both pointers are non-null. It copies no text and provides no implicit
accept/cancel semantics. If desired, bridge it in application code:

```cpp
void numberEntered(Numkey* keypad) {
    // Application owns validation and any target text assignment/repaint.
    keypad->targetEventHandler();
}
// Register separately from the target Button's own callback:
keypad.setEventHandler(numberEntered);
keypad.show(&targetButton);
```

`reset()` sets `entry` to zero only; the text and target pointer remain. Zero is
a valid numeric input, so `entry == 0` alone is not an unambiguous completion
signal. `Numkey.h`'s historical `show(Button*)` comment should not be interpreted
as an automatic callback bridge. This work documents actual behavior rather
than introducing a different acceptance protocol.

Coverage: `tests/host/test_legacy_input_contracts.cpp` uses different target and
keypad text, a real `=` touch, callback counts, a debounced repeated touch,
direct bridging, bridging from the keypad callback, reset, and null-target /
null-target-handler no-ops. Target text stays unchanged throughout.

## Dial: ring and tick colors have different roles

For `setColors(background, foreground, border)`:

- The outer filled circle (the visible ring after the inner face is painted)
  uses **`fgColor`**, not `borderColor`.
- The inner face uses `bgColor`.
- Ordinary ticks and scale endpoint text use **`borderColor`**.
- Setpoint/high/low markers use their dedicated colors. Needle/value color can
  use the foreground or alarm-limit colors; these are separate from the ring.

`legacy_input_contracts` records actual circle, horizontal/vertical line-run,
and text operations for two contrasting palettes with foreground and border
colors swapped. It disables value text and optional limit/setpoint markers,
uses a mid-scale value, and checks ring and tick pixel witnesses as well as
operation colors. No Dial production change is necessary.

## IconButton: explicit repaint remains application-owned

`IconButton` inherits `Button::checkTouch()`: install a `Button*` callback or
there is no touch-state transition. A qualifying, debounced touch toggles
`touched` and calls the handler; it does not automatically repaint the icon.
Use `button.update()` explicitly or call virtual `update()` in that handler:

```cpp
void iconTouched(Button* button) {
    button->update();
}
icon.setEventHandler(iconTouched);
```

This is toggle/level sampling, not a release-edge state machine. Respect the
strict debounce comparison; setting a zero interval still does not admit two
events in the same millisecond. `update()` selects the pressed bitmap only when
`touched` is true and a pressed bitmap exists; otherwise it uses the unpressed
bitmap. Transparent pixels preserve the previous background rather than
restoring it automatically.

The independently maintained `tests/host/test_icon_contract.cpp` covers these
IconButton contracts; see also [designer widget findings](designer-widget-findings.md)
for the coordinated widget audit. This document does not claim ownership of
those production or test changes.
