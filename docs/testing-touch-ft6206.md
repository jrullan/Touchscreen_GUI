# FT6206 adapter: host evidence and validation limits

`TouchDriver_FT6206.h` is an additive, polling-only `TouchDriver` adapter;
legacy `Canvas_FT6206` is unchanged. It owns the bundled `Adafruit_FT6206`
by value, with no adapter heap allocation or IRQ registration. A borrowing
`TouchMapper` must be destroyed before its adapter. Copies/moves are disabled
by the `TouchDriver` base; destruction destroys the controller once.

## API and readiness

- Constructor: optional `uint8_t threshold = FT62XX_DEFAULT_THRESHOLD` (128),
  exactly the threshold used by the legacy canvas. Construction performs no I2C.
- `begin()`: one bundled `begin(threshold)` attempt per failed call, retryable;
  successful initialization is idempotent. The adapter adds no wait/retry loop.
  The bundled implementation itself starts Wire, sets the threshold, and checks
  vendor/chip IDs; this is not a timeout guarantee for underlying I2C operations.
- Before successful begin, `touched()` is false, `touchCount()` is zero, and
  `readRaw(x,y,z)` writes three zeros without accessing hardware.
- After begin, `touched()` and `touchCount()` query the real current touch count;
  the bundled driver returns 0/1/2 and normalizes counts above 2 to zero.
  No cached count is promoted into a synthetic event.
- `readRaw` makes one `getPoint(uint8_t(0))` call and passes through x/y/z.
  An explicit index is essential: the bundled header declares both
  `getPoint(uint8_t n = 0)` and `getPoint(void)`. An unqualified no-argument call
  on the concrete controller is ambiguous. The point is the first touch only,
  even when `touchCount()` reports two touches.
- `needsFiltering()` is false: the capacitive controller returns resolved
  coordinates with binary contact z, not resistive ADC samples. No extra
  three-read median is imposed; this is a policy, not measured noise evidence.

Touch count and point acquisition are separate I2C transactions, not an atomic
snapshot. Contact can disappear between them; the adapter passes through the
bundled zero point rather than inventing contact or reusing old coordinates.
The existing mapper decides gesture state from `touched()`, not z, so this
pre-existing race is not fixed by this additive adapter.

## Integrated nominal calibration default

Source inspection: `Adafruit_FT6206.cpp::readData()` decodes each axis from
12 bits, so the representable controller range is **0..4095**, safely fitting
`int16_t`. It does not enforce a particular panel extent. Nominal **0..239 x
0..319** is a 240x320 panel assumption, not a measured electrical calibration.

Integrated `TouchCal::FT6206_DEFAULT` fields in `TouchCalibration.h`:

```cpp
{239, 0, 319, 0, false}
// rawMinX, rawMaxX, rawMinY, rawMaxY, swapXY
```

Descending endpoints preserve the legacy portrait direction on both axes,
while mapping to inclusive valid pixels 0..239 and 0..319. Legacy portrait
uses `240 - rawX`, `320 - rawY`; copying those extent values literally yields
out-of-bounds 240/320 at raw zero. The integrated default deliberately corrects
that one-pixel endpoint error; it is not bit-for-bit legacy output. With this
mapper, historical landscape (physical rotation 3) has direction
`(rawY, 239 - rawX)`, corresponding to legacy `(rawY, 240 - rawX)` with the
same endpoint correction. Out-of-range samples are clamped by the mapper,
not modified by the adapter. Confirm actual panel ranges/orientation physically
before claiming this default calibrated.

## Reproduce host tests

From the worktree:

```sh
python3 tests/host/run.py touch_ft6206
python3 tests/host/run.py
```

The runner compiles real GUI/mapper translation units with ASan, UBSan and leak
detection. The test locally suppresses the bundled hardware header with its
actual include guard and substitutes a faithful Touch-derived controller API;
no shared runner/stub changes are needed. It is not an I2C emulator and does not
compile or validate the real FT6206 implementation.

Coverage: constructor/default and custom threshold forwarding; one attempt per
failed begin, retry and successful idempotence; zero/no-access guards; value
ownership and teardown; borrowed mapper lifetime; live 0/1/2 count and invalid
count normalization; raw endpoints and 12-bit maximum pass-through; zero-point
pass-through; all four mapped corners at all four rotations; one sample per
poll; release/idle and immediate repress without reads on zero touch count.

TDD evidence: initial run failed because the new header did not exist; minimal
readiness implementation then passed lifecycle checks. Subsequent runs failed
at filtering policy, live-count access, and raw pass-through assertions before
the corresponding implementations, followed by a passing targeted sanitizer
run. Corner/lifetime integration tests were then added as regression coverage
and passed without changing mapper production code. The complete host suite
also finished successfully, including the FT6206 and concurrently added STMPE610
adapter tests, with ASan/UBSan/leak detection enabled.

No FT6206 hardware was available. No Arduino build, flashing, serial access,
I2C/electrical validation, noise measurement, or physical touch/orientation
acceptance was performed by this task.
