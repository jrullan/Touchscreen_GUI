# Hardened Keyboard: contract and host verification

## API and ownership

`Keyboard` derives from `Button`; include `Keyboard.h`. Both the default constructor
and `(unsigned width, unsigned height, int background, int foreground, int border)`
constructor remain available. Type ID is `0x36` (not Popup's `0x33`). Default geometry
is 240x192, font size 1, inherited border width 2, and `autoremove=false`.

The keyboard owns `KEYBOARD_BUF_SIZE + 1` inline bytes (default 49 including NUL).
Set the same macro for **all** translation units; supported capacities are 1..255,
checked at compile time. `getText()` intentionally returns a **read-only borrowed
view**, unlike v2's mutable pointer. `getTextSize()` is the current byte count,
not capacity. Views remain valid until the keyboard dies, but edits change contents.
ASCII printable bytes 32..126 only: `append(char)` ignores other bytes and full-buffer
appends; `setText(const char*)` validates before changing anything, rejects oversize
or nonprintable input atomically, permits overlapping input, and treats null as empty.
Input pointers must reference readable NUL-terminated strings (or at least capacity+1
readable bytes when rejecting an oversized string). `deleteChar()` on empty is safe.
`clear()` preserves modes/session; `reset()` clears text and modes, detaches the
borrowed target, and returns to `Editing`, including recovery from invalid `show(target)`.
Edits change the buffer; call `update()` to redraw programmatic edits. Touch edits
redraw automatically. `show()`, `update()`, and `drawFrame()` remain unconditional
full keyboard repaints (when drawable), including for external damage/overlay
restoration; no committed-frame cache changes their public behavior.

Ordinary character, Space, and backspace touches repaint **only the text-field
interior**, and only if the buffer length changes. Its border, backspace key, and
all other keys are untouched. The entire interior is erased before drawing the
new centered suffix, so shorter/empty text cannot leave old glyphs. Full-capacity
character/Space touches, empty backspace, and text-display taps consume input but
produce no drawing. Shift/Symbols intentionally still perform a full repaint to
refresh key labels/highlights; these mode changes are distinct from ordinary typing.
There is no extra allocation, framebuffer, or retained display cache.

The inline editing path allocates nothing. Inherited `Button` still attempts its
normal owned `Text` object and 9-byte text buffer allocations during construction;
the keyboard works even if either allocation fails. It neither borrows nor replaces
that owned storage. No framebuffer, child-key widgets, IRQs, or edit-time allocations.
The host test reports `sizeof(Keyboard)=192`, `sizeof(Button)=104` on this 64-bit host;
these are **not MCU ABI sizes**. A repaint also uses a capacity+1-byte stack string.
Two static 4x11 character maps occupy 88 bytes, plus small label literals.

`show(Button*)` borrows a target for the editing session and copies its initial text.
The target and its original owned Text storage must outlive the session. A null/self
pointer, missing Text/buffer, unterminated capacity-bounded contents, nonprintable
contents, or text larger than keyboard capacity leaves the keyboard hidden and
inert, with no target retained and no silent truncation. `reset()` or a valid
`show(target)` recovers. `show()` never adds a registry entry and preserves pending
text/modes; it reopens a valid session as `Editing`. `hide()` preserves pending text
and the current session/target. Do not destroy a hidden session's target before
`cancel()`, `accept()`, or `reset()`.

`accept()` first checks the target's existing nonnull Text/buffer and capacity;
it then copies through the existing Text setter without allocation. Failure returns
false and leaves target/text/status/visibility/callback unchanged. Success returns
true, sets `Accepted`, clears the borrowed target, hides/restores underlying widgets,
and optionally unregisters **this keyboard**, never the registry's unrelated tail.
`cancel()` does not change target contents; it sets `Cancelled` and performs the same
cleanup. Repeated completion does nothing until reopened. Completion works without
a handler. `setEventHandler(void (*)(Keyboard*))` runs only after accept/cancel,
not for characters/modes/backspace. It can inspect `result()` and `getText()`;
`targetButton` is already null. It is the final action: the callback may remove/delete
the keyboard. Registries are non-owning: enable `autoremove` or remove the entry before
deleting a registered keyboard. Borrowed target Buttons are never deleted.

`hide()` sets visible false **before** restoration and does nothing drawable when
already hidden, unattached, or invalid-layout. Restoration is a full clipped Canvas
background + current Screen's background/visible children + Canvas visible widgets.
It intentionally does not call `Canvas::redraw()`: `Screen::show()` forcibly makes
children visible, which would resurrect a Screen-registered keyboard. Autoremove
checks Canvas's registry first, then its current Screen's registry. An inactive
Screen's entries must be removed by that Screen's owner before deletion. Generic
Widget/Button/Canvas/Screen lifecycle APIs are unchanged; use `Keyboard::hide()`
(not the nonvirtual inherited `Widget::hide()`) for keyboard restoration.

## Layout and touch

The explicit six-row, ten-column layout is a documented v2 deviation:

- Row 0: text display spanning columns 0..8; backspace at column 9.
- Rows 1..3: ten character keys each.
- Row 4: Shift at column 0, eight character keys, Symbols at column 9.
- Row 5: Space at columns 0..5, Cancel at 6..7, OK at 8..9.

Normal rows are `qwertyuiop`, `asdfghjkl` plus backtick, `zxcvbnm,./`, and
`-_=~?!:;` between the commands. Shift capitalizes letters. Symbol rows are
`1234567890`, `!@#$%^&*()`, `-_=+[]{}` plus backslash and pipe, and `;:'\",.<>`
between the commands. All 95 printable ASCII characters are reachable across modes.
Shift is retained while symbols are active but does not transform symbols.
Commands are a typed action attached to a cell, never a character-value dispatch;
literal `#`, `^`, `<`, and backslash are ordinary symbol entries. Command labels
`^`, `#`, `<` may match characters without stealing their input behavior.

A valid cell conservatively requires `floor(w/10) >= 18*fontSize+2*borderWidth`
and `floor(h/6) >= 8*fontSize+2*borderWidth`; this ensures the six-character Cancel
label fits its two-cell region with actual classic GFX 6x8 font advances. Keyboard
labels and buffer text always use that classic font, even when the shared TFT has
an active custom font. A stack-local, framebuffer-free `Adafruit_GFX` adapter emits
explicitly positioned glyphs to the TFT; it does not change the TFT's font, text
size/color, cursor, or wrapping. Both real GFX `drawChar` overloads consult their own
`gfxFont`, so calling them directly on the shared TFT would not enforce this policy.
The adapter uses GFX's existing font table: no copied font, heap allocation, retained
widget state, or pixel buffer is added. Restoration still invokes other visible
widgets under their existing drawing contracts.
Default style minimum is **220x72**. No adaptive font changes are made. Font zero,
negative/extreme borders, nonpositive or too-small geometry are fully inert.
Wide arithmetic precedes clipping/narrowing even for public INT_MIN/INT_MAX positions
and dimensions. Fills are clipped to the physical TFT; partially clipped labels are
omitted rather than letting GFX wrap them or paint glyphs outside their cell.
The display shows the tail that fits in its region. Proportional cell boundaries
and their exact inverse include remainder pixels without out-of-range indexes.

Touch is pass-through (`true`) for null/unattached/hidden/invalid/outside points,
including off-panel points. Every inside point consumes (`false`), including blank
display cells and debounce-suppressed touches, even without a handler. Held touch
repeats at the inherited debounce interval (default 500ms); there is no release-edge
requirement. Elapsed-time subtraction uses unsigned rollover-safe arithmetic.

## Tests

```sh
python3 tests/host/run.py keyboard
python3 tests/host/run.py
python3 tests/rasterized/run_keyboard.py
python3 tests/rasterized/run.py
python3 tests/rasterized/run_widgets.py
python3 tests/transactions/run.py
python3 tests/xpt2046/run.py
python3 tests/host/check_returns.py
```

`tests/host/test_keyboard.cpp` tests capacity/atomic rejection/overlap/delete/control
filtering, all printable character keys and typed commands, all remainder pixels in
an uneven-width QWERTY row, draw-operation bounds under adversarial geometry, invalid
styles, hidden/unattached/null/edge touch, held/debounced/unsigned-wrap input,
target accept failure/success/cancel, Text object and backing-buffer OOM, allocation
counts, repeated exact-entry autoremove, Screen hide restoration, and callbacks
that delete the keyboard from the actual OK/Cancel touch path. ASan/UBSan/leak checks
run against real production translation units. The older raster runners also compile
Keyboard.cpp with installed real Adafruit_GFX, not only the no-op text stub. Their
existing pixel assertions test the older widgets; keyboard-specific raster pixel
checks and Arduino compile-matrix integration are maintained separately by the
integration owner. No hardware/Arduino build/flash is claimed by these host runs.

The inherited-custom-font regression was reproduced before the fix using the real
GFX runner: FreeSans18pt7b, keyboard `(10,70,220,192)`, 32 `M` bytes, unexpected
white pixel `(16,57)` outside the keyboard. After the fix all six keyboard raster
cases pass, including both wrap settings in the edge cases. A supplemental real-GFX
scratch probe also verifies exact classic keyboard pixels and unchanged caller
custom-font bounds, size, color, cursor, and wrapping after show/update (both wrap
settings); it does not replace or weaken the retained regression. The full host
ASan/UBSan/leak suite, older raster runners, transaction suite, actual bundled
XPT2046 tests, and strict return/const/sentinel checks pass. The minimal host GFX
boundary stub remains intentionally non-rasterizing.

## Flicker regression (strict RED/GREEN)

The first added host regression drives actual character, Space, and backspace
hits at uneven 231x197 geometry. It requires every recorded fill to stay strictly
inside the text display border, with the first fill erasing the complete interior,
and checks unchanged edit allocation counts. Before the fix,
`python3 tests/host/run.py keyboard` failed at `op.x>=left && op.y>=top`: the
ordinary character path repainted the text border and all keys through `update()`.
After adding the interior-only helper/path, it passed.

A second regression then failed at `before==k.getText() && c.tft->ops.empty()`
for empty backspace. Restricting repaint to a changed buffer length (or an explicit
mode change) made empty backspace, display taps, and full-capacity character/Space
hits pass with zero fills. Explicit `update()` is separately required to draw.
The host boundary does not rasterize glyphs; these assertions prove fill locality,
not physical flicker timing. The six real-GFX keyboard cases (`labels`, `restore`,
`edges`, `modes`, `customfont`, `font0`) and the complete host suite all passed
under ASan/UBSan/leak detection after the fix. No hardware access, Arduino build,
flash, commit, or push was performed for this change.

Host-only memory probe: `sizeof(KeyboardGlyphs)=48`, `sizeof(Adafruit_GFX)=40`, and
`sizeof(Keyboard)=192` bytes. The temporary adapter adds stack state per painted
label, with no change to Keyboard object size; these are not MCU ABI or measured
peak-stack figures. No Arduino compile, device run, commit, push, or flash was done
for this fix.

