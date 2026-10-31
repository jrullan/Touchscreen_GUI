# Phase 0/core regression log

Base: merged upstream PR #2 (`46b03f5`; same tree as `afd843c`).

Executed locally with g++ and ASan/UBSan/LSan through `python3 tests/host/run.py core`.

Vertical red/green results:

1. Unattached Widget::hide: UBSan reported null Canvas member access. Initialize geometry/context, guard drawing, and mark invisible. Subsequent run passed this assertion and reached the next failure.
2. Unknown Widget::getType: UBSan reported reaching the end of a non-void function. Add deterministic unknown/null fallback. Subsequent run reached the numeric edge case.
3. INT_MIN conversion: UBSan reported signed negation overflow. Compute unsigned magnitude; full core test passed.
4. Added null text-length input: UBSan reported null load. Add null guard; full core test passed.

Strict compiler return gate `python3 tests/host/check_returns.py`:

- RED: GUI_TFT.cpp drawNumber missing return under -Werror=return-type.
- Fix: explicit legacy success return (0).
- RED: Seeedtouch.h analog begin missing return.
- Fix: explicit true (analog backend has no hardware initialization transaction).
- GREEN: strict return gate exits 0.

Host stubs record draw operations and maintain a raster for primitives, but text records bounds/content rather than rasterizing the actual Adafruit font. These tests cannot establish physical display appearance or SPI timing. Host int/long/pointers differ from MCU ABI; Arduino builds and optional board instrumentation are separate gates.

C allocation failure injection is available via `allocation_hooks.h`, using link-time wrapping of malloc/calloc/realloc. It does not pretend to intercept every allocation inside the C++ runtime or device driver.
