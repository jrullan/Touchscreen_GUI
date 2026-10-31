# Nested SPI transactions

## Public API and integration

```cpp
{
    Guitft::ScopedWrite render(*canvas.tft);
    // Bounded TFT-only drawing. Nested ScopedWrite guards are supported.
    canvas.tft->fillRect(x, y, width, height, BLACK);
    canvas.tft->drawString("value", x, y, 1, WHITE);
} // closes the physical transaction, including on an early return
```

`Guitft::startWrite()` / `endWrite()` implicitly override the real Adafruit
virtual methods. Their outer boundary uses explicitly qualified
`Adafruit_ILI9341::startWrite()` / `endWrite()` calls, avoiding recursive virtual
dispatch. The omitted `override` keyword permits the existing legacy host panel
fixture's nonvirtual API; the isolated test instead compiles real Adafruit_GFX
and checks dispatch through an `Adafruit_GFX&`.

The guard is public, noncopyable and nonmovable. Allocate it on the stack as a
usage convention (the type does not prohibit heap allocation). It holds a reference;
the display must outlive it. Gauge/Slider and Terminal now use bounded guards
without changing graphics primitives.

Keep each guard around **bounded display-only work**. Do not hold a TFT transaction
across touch or another SPI device, HTTP/network calls, yield/task handoff,
callbacks that may access other SPI devices, or unbounded loops. This is not a
thread/ISR synchronization mechanism. Every manual begin must have one end; do
not manually end a guard's begin. Do not copy/move a live display object.

Only pixel-drawing calls belong inside a guard. Run `begin()`, `setRotation()`,
`invertDisplay()` and scrolling/control commands outside it: driver `sendCommand`
uses direct CS/transaction boundaries that bypass the nesting counter and can
deselect the display prematurely. Backend-internal `optimistic_yield` is still
possible during fills; coordinate callbacks/tasks using other devices on the same
SPI bus separately. This guard neither suppresses those yields nor locks the bus.

## Counter contract

- Transaction state uses an initialized `uint32_t` depth and a sticky overflow
  flag; neither needs dynamic allocation.
- Balanced nesting produces one physical begin/end pair. Intermediate ends leave
  CS selected. An unmatched end at zero is ignored.
- Depth `UINT32_MAX` is still representable and can unwind normally. A further
  start latches overflow without wrapping. All subsequent ends are ignored:
  **fail closed**, never release a potentially live transaction early. This is
  a programming-error state requiring device reset/reconstruction, not a normal
  recovery API. Calling inherited `begin()` alone does not clear the latch.
- No destructor-forced close or implicit bus takeover is added.

`__STM32F1__` keeps the existing `Adafruit_ILI9341_STM` inheritance. The wrapper
start/end and guard are no-ops there: no unverified transaction methods are
called on that legacy backend, and no batching performance is promised. A real
STM32 driver/toolchain is not installed here, so target compilation remains
unverified. A future backend-specific implementation must first establish its
actual transaction API.

## Isolated host regression

```sh
python3 tests/transactions/run.py
python3 tests/host/run.py
python3 tests/rasterized/run.py
```

The new runner uses the installed Adafruit_GFX.cpp, Adafruit_GFX.h, and real font.
Only the panel/bus boundary is replaced with an instrumented panel that brackets
its drawPixel writes as Adafruit_SPITFT does. It does **not** compile the physical
SPI driver or measure electrical CS. The test counts boundary begin/end calls,
selected state, pixel writes, and output pixels under ASan/UBSan/leak detection.
Shared host/rasterized stubs are unchanged. Set `ADAFRUIT_GFX_DIR` to select a real
GFX source directory; a missing dependency reports SETUP REQUIRED, not PASS.

Coverage: real GFX drawRect virtual dispatch and its nested line/pixel fallbacks;
unmatched ends followed by a real drawCircle; overflow latch and exact maximum
via private-state boundary injection; normal/nested/early-return guards;
fillRect, borders, circles, existing run-length drawLine, strings and numbers;
and 1001 nesting levels (well past the old byte counter range).

### Executed evidence

Strict test-first sequence was observed:

1. Real-GFX drawRect boundary count assertion failed before production changes;
   passed after adding outermost dispatch.
2. Unmatched ends followed by drawCircle failed before the zero guard; passed
   after the guard.
3. Overflow assertion failed before the overflow latch; passed afterward.
4. ScopedWrite tests failed to compile because the public API was absent; passed
   after adding the guard.

All transaction cases, all host suites, and both rasterized cases passed.
Real-GFX **count benchmark** from this fixture: the same drawRect produced
47/47 boundary begin/end calls without nesting versus 1/1 with nesting, with
42 pixel writes and identical pixel buffers. This is a host fallback-path count,
**not** a hardware timing/speedup benchmark; real SPITFT has optimized primitives
and may have different baseline counts.

An actual clean ESP8266 build passed using core 3.1.2 and this exact worktree:

```sh
/home/jrullan/dev/tft-panelboard/tools/arduino-cli compile --clean \
  --fqbn esp8266:esp8266:d1_mini \
  --library /home/jrullan/dev/Touchscreen_GUI-v2-ports \
  --build-path "$TMPDIR/spi-transactions-esp8266-build" \
  --output-dir "$TMPDIR/spi-transactions-esp8266-out" \
  tests/arduino/display
```

Reported usage: RAM 29136/80192 bytes, IRAM 60019/65536 bytes, flash code
249952/1048576 bytes. These are whole-sketch totals, not transaction-only costs.
No flash/upload, hardware access, commit or push was performed.

## Runner / CI integration

CI runs `python3 tests/transactions/run.py` and
`python3 tests/rasterized/run_widgets.py` against pinned Adafruit GFX after its
checkout. The latter covers Gauge/Slider and Terminal real glyphs and nested bus
boundaries. The host runner discovers only tests/host, not these isolated suites.
Do not add their panel stub to shared include paths. Continue using clean Arduino
builds after the Guitft layout change.
