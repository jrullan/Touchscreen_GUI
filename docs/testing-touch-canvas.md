# Opt-in mapper-backed Canvas and XPT2046 adapter

## API and ownership

```cpp
#include "TouchDriver_XPT2046.h"
#include "Canvas_Touch.h"

// Declaration/destruction order matters: driver outlives mapper; both outlive canvas.
TouchDriver_XPT2046 driver(TOUCH_CS); // polling only; IRQ must be 255 (default)
TouchMapper mapper(&driver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_LANDSCAPE, BLACK, &mapper, TFT_CS, TFT_DC);

// Constructor attempts startup. An unsuccessful startup remains unavailable to scan.
if (!canvas.isReady()) canvas.init(); // retry later; never busy-waits
canvas.scan();
canvas.setRotation(2); // actual ILI9341 rotation, normalized modulo 4
```

The driver owns its bundled `XPT2046_Touchscreen` **by value**: no hardware heap
allocation, no hardware-allocation failure path, and no allocation/leak on begin
failure/retry. It invokes the bundled `begin(0)` until successful, then never
begins again. Before successful begin, `touched()` is false and `readRaw()` returns
zeros without accessing hardware. Any IRQ argument other than 255 is unsupported:
`begin()` returns false before hardware startup, preventing unsupported callback
lifetimes in the bundled driver. Raw axes/pressure are passed through. Calibration
belongs to the mapper. `needsFiltering()` returns false: bundled XPT acquisition
already applies closest-pair averaging and caches successful samples for 3 ms.
The existing default calibration constants are unchanged.

`Canvas_Touch` borrows its mapper, which borrows its driver. Neither is deleted by
the canvas. All three types are noncopyable. Do not share one mapper between active
canvases: polling state and orientation are mutable. Do not replace the public legacy
`ts`/`tft` pointers or delete the borrowed mapper/driver while the canvas is usable.

The canvas owns the TFT and a tiny `Touch` compatibility bridge. Until startup
succeeds, it owns the unpublished bridge privately and leaves `ts == nullptr`.
On success, ownership transfers to base `Canvas::ts`; the derived destructor only
deletes an unpublished bridge, and the base destructor deletes the published one
and the TFT. There is no double-delete. Failed TFT allocation is retried by a later
explicit `init()`. Failed bridge allocation is retryable; a failed mapper begin
retains the same bridge/TFT. Successful display initialization is not repeated on
retry, and successful canvas startup makes subsequent `init()` a no-op.

## Sampling, propagation, and rotations

`Canvas::scan()` is unchanged. Its strict elapsed-time scan gate still runs before
`ts->touched()`, then `getTouchedPoint()`, bounds checking, reverse-order canvas
widgets, and (unless blocked) screen widgets. The private bridge's `touched()` means
**ready to sample**, not physically pressed; it never reads hardware. The point
method honors the existing strict `TOUCH_SAMPLING_TIME` gate and polls the mapper
exactly once per eligible point sample. PRESSED and HELD dispatch; RELEASED/IDLE do
not dispatch. Calling `getTouchedPoint()` directly uses the same point sampling gate
but, as with legacy point getters, bypasses the outer scan gate.

Before touch begin and each mapper poll, the canvas explicitly drives its supplied
TFT CS HIGH. Call scan outside drawing/SPI transaction scopes; this does not arbitrate
another device's live SPI transaction. Supply the real TFT CS pin (not the default
`-1`) when display and touch share SPI. Host tests observe the explicit deselection
call before the fake driver is accessed; they do not establish electrical timing.

The only base API change is making `Canvas::portrait()` and `landscape()` virtual.
Their legacy implementations are unchanged. New canvas overrides synchronize the
mapper, TFT, legacy layout parity, and `w/h`, even via a `Canvas*`. Portrait selects
physical rotation 0; landscape and legacy constructor mode 1 select rotation 3.
`setRotation(0..3)` selects all four physical rotations: even rotations are 240x320,
odd rotations 320x240. Existing bounds checking uses synchronized layout parity.
Use the canvas rotation methods, not direct `canvas.tft->setRotation()` or direct
mapper orientation changes, to keep them synchronized. Rotation does not redraw or
reflow existing widgets.

## Host verification

```sh
python3 tests/host/run.py touch_canvas
python3 tests/host/run.py
```

`test_touch_canvas.cpp` is automatically discovered by the existing runner. It
compiles production Canvas, TouchMapper, and the inline XPT adapter. A local
include-guard seam substitutes only the hardware XPT object; shared stubs and runner
are untouched. ASan/UBSan/leak detection cover null mapper, failed begin, retry,
repeated init, pre-begin access, unpublished/published cleanup, borrowed stack
lifetimes, TFT/bridge allocation failure, mapped dispatch, held/released/idle,
scan and point sampling gates, propagation order/blocking, shared-CS deselection,
all rotations/bounds, and base-pointer orientation helpers.

Observed TDD failures included missing adapter/canvas headers, absent point dispatch,
failed TFT-OOM recovery, missing rotation API, and unsynchronized base-pointer
portrait changes; each was followed by a passing targeted run. The full host suite
passed. A separate two-translation-unit probe included both new headers, linked the
real core, and ran successfully under `-std=c++11` (the pre-existing host stub's
C++17 inline-variable extension warning was disabled).

## Still requires board verification

No board compile, upload, or flash is performed by these tests. Verify the actual
bundled XPT polling constructor/begin, real SPI CS/transaction timing,
calibration/pressure/noise, all four on-device corner mappings, and widget redraw
behavior on the target board. Legacy XPT/FT6206/STMPE610 canvases have not been
migrated or otherwise changed.
