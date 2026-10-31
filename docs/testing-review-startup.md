# Review regressions: Slider visibility and touch startup

## Run on the host

From the repository root:

```sh
python3 tests/host/run.py review_startup lifecycle visibility
python3 tests/host/run.py
```

The unchanged runner discovers `test_review_startup.cpp`, compiles the real core
translation units with ASan/UBSan, and enables leak detection. This test includes
`lifecycle_drivers.h` itself: only hardware Touch subclasses and the existing
Arduino/display boundary are substituted. No force-include or runner change is
needed. The existing `lifecycle` target additionally links its second translation
unit for header ODR coverage.

After building, individual startup cases can be run with:

```sh
"${TMPDIR:-.host-build}/touchscreen-gui-tests/review_startup" slider
"${TMPDIR:-.host-build}/touchscreen-gui-tests/review_startup" propagation
"${TMPDIR:-.host-build}/touchscreen-gui-tests/review_startup" allocation
"${TMPDIR:-.host-build}/touchscreen-gui-tests/review_startup" xpt_begin
"${TMPDIR:-.host-build}/touchscreen-gui-tests/review_startup" ft_begin
"${TMPDIR:-.host-build}/touchscreen-gui-tests/review_startup" stm_begin
"${TMPDIR:-.host-build}/touchscreen-gui-tests/review_startup" failed_destruction
```

## Recorded RED / GREEN evidence

- **Slider RED:** the attached Slider hide/show test aborted on
  `assert(slider.visible)`. `Widget::hide()` makes the widget invisible, and the
  guarded drawing methods cannot draw until visibility is restored.
- **Slider GREEN:** setting `visible = true` at the beginning of `Slider::show()`
  passed the same test, including both border and fill drawing assertions.
- **Startup RED:** XPT2046 and FT6206 failed the no-premature-read assertions.
  A dedicated rerun failed on `LifecycleDriver::touchCalls == touches`, proving
  `Canvas::scan()` queried a driver whose `begin()` had returned false. Direct
  `getTouchedPoint()` also returned a point before successful startup.
- **STMPE610 RED:** the false-begin case terminated with SIGALRM under the test's
  three-second watchdog because the adapter loop retried forever.
- **Startup GREEN:** XPT2046 and FT6206 isolated begin cases passed after guarded
  publication; the complete startup suite passed after STMPE610 was changed to
  one begin attempt per `init()` invocation.
- **Final shared regression run:** `python3 tests/host/run.py` passed `charts`,
  `core`, `lifecycle`, `new_oom`, `review_ranges`, `review_startup`, `text`, and
  `visibility`, with ASan/UBSan and leak detection enabled.

An initial build overlapped another agent's Indicator overload edits and failed
with an undefined `Indicator::setNum(unsigned int)` reference. Once that edit
completed, the unmodified build command linked and exposed the actual Slider
assertion. That linker failure is not counted as RED evidence.

## Startup contract under test

- `host_nothrow_new_fail_after = 1` lets the base Canvas allocate its display and
  fails the next nothrow allocation, the touch driver. The test verifies that the
  hook fired, no touch driver was constructed, and both scan and point queries
  remain inert. All four actual adapters then retry successfully on `init()`.
- For XPT2046, FT6206, and STMPE610, a failed-begin driver remains privately owned
  as `pendingTouch`. The public `ts` remains null until begin succeeds, so even
  nonvirtual `Canvas::scan()` invoked through `Canvas&` cannot query it. An
  adapter-only method hiding `scan()` would not protect that base-reference path.
- Repeated failed begin attempts preserve the exact allocated driver and display
  addresses. A new-allocation failure is armed during a failed-begin retry and
  remains unconsumed, proving that retry needs no replacement touch allocation.
  Success publishes that same driver as `ts`; later `init()` calls neither
  replace it nor call begin again.
- Failed startup destruction is exercised through `Canvas*`: the derived
  destructor releases a pending driver; the base destructor releases the display.
  Successful startup transfers touch ownership to the existing base destructor.
- STMPE610 now returns from a failed begin and requires an explicit later
  `init()` call to retry, rather than waiting indefinitely in the adapter.
- SEEEDTOUCH has no begin phase in its actual adapter. Its driver construction
  is its startup step; only allocation-failure/retry coverage applies. A fake
  begin-failure test for that adapter would invent behavior it does not use.

## Deliberately retained behavior and limits

- **Handlerless Slider propagation is deferred, not redesigned.**
  `checkTouch()` still returns false with no handler, including outside the
  Slider bounds and for either `block` setting. In Canvas reverse-order
  dispatch this stops delivery to lower widgets. The regression test explicitly
  locks down that behavior, unchanged CV, and absence of drawing; hiding the
  Slider allows the lower widget to receive the touch again.
- Slider debounce still uses addition-based elapsed-time comparison. This work
  does not redesign debounce wraparound, geometry/ranges, or event return-value
  semantics for handled touches.
- A failed adapter startup now exposes `ts == nullptr` rather than exposing an
  unstarted driver. Applications must not use or replace private pending state;
  public driver-pointer reassignment and post-success manual driver deletion are
  not supported retry mechanisms. Use adapter-owned startup and `init()` retries.
- Retrying startup still reinitializes/clears the display and resets sampling
  timestamps. This is existing behavior; retry scheduling/backoff and a public
  status/result API remain outside this change.
- Removing the adapter's STMPE610 retry loop does not bound the duration of a
  hardware display/driver call. Lower-level SPI/I2C waits and other blocking
  hardware behavior remain unverified and deferred.
- Host substitutes prove adapter ownership, call ordering, and retry behavior,
  not board startup, electrical behavior, touch calibration, or real controller
  failure recovery. No device was flashed and no hardware validation is claimed.
