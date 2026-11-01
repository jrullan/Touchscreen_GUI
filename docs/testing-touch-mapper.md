# Hardened additive touch mapper: host tests and contract

## Scope

`TouchDriver.h`, `TouchCalibration.h`, and `TouchMapper.h/.cpp` introduce an additive
v2-inspired mapping layer. Existing legacy Canvas_XPT2046, FT6206, STMPE610 and
legacy Touch/calibration APIs are not replaced by this slice. No panelboard was
flashed, no Arduino build was run, and no hardware/overnight heap claim is made.

## API and lifetimes

- `TouchDriver` has a virtual destructor, pure `bool begin()`, `bool touched()`,
  and `void readRaw(int16_t&, int16_t&, int16_t&)`. Defaults are
  `needsFiltering() == true` and `touchCount() == (touched() ? 1 : 0)`.
- Drivers and mappers are noncopyable (and cannot be implicitly moved).
  `TouchMapper(TouchDriver*, const TouchCalibration&)` borrows its driver; the
  driver must outlive the mapper. Neither construction nor destruction allocates
  or deletes hardware. Use one active mapper per driver; independent mappers
  polling one device would consume each other's samples.
- `bool begin()` validates both initial calibration spans before calling hardware.
  Null/invalid/failed startup leaves `isReady()` false; `poll()` makes no driver
  calls until successful startup. Failed startup is retryable, successful startup
  is idempotent. `begin()` does not poll or read samples.
- `bool setCalibration(const TouchCalibration&)` rejects either zero span without
  modifying valid calibration, readiness, or gesture state. A valid assignment
  (even the same calibration) resets the gesture to IDLE, both points to zero,
  and elapsed time to zero; it does not restart hardware.
- `void setOrientation(uint8_t)` normalizes modulo four. A change of effective
  rotation resets the gesture in the same way; setting the same rotation is a
  no-op. A finger still down on the next poll starts a fresh PRESSED gesture.
  This prevents press/current coordinates from using different coordinate systems.

`TouchCalibration` retains the v2 fields `rawMinX/rawMaxX/rawMinY/rawMaxY/swapXY`
and signed 16-bit endpoint types. `TouchCal::XPT2046_DEFAULT` is exactly
`{3894, 261, 3891, 320, true}`.

### Coordinates and filtering

Axis swap precedes calibration. Signed 64-bit intermediates widen **before**
subtraction and multiplication. Ascending and descending spans, including the
full signed 16-bit range, are supported. Integer division truncates toward zero;
portrait coordinates are clamped to x=0..239, y=0..319 before rotation:

| Rotation | Output |
| --- | --- |
| 0 | px, py |
| 1 | py, 239-px |
| 2 | 239-px, 319-py |
| 3 | 319-py, px |

One touched poll reads three samples when filtering is needed, taking independent
medians of x, y **and z**. Already-filtered drivers are read exactly once. Sample
storage starts at zero, so a driver that omits z does not leak uninitialized data;
drivers should nevertheless fill every output. Pressure is retained, not used as
an extra touch threshold: `touched()` determines contact.

### State and duration

The state is deterministic IDLE before startup. A down poll changes IDLE or
RELEASED to PRESSED, capturing `pressPoint()` and the current `millis()`; subsequent
down polls become/stay HELD and update only `currentPoint()`. Up changes PRESSED
or HELD to RELEASED, then a second up poll becomes IDLE. RELEASED followed by down
starts a new press immediately (no discarded poll). Release keeps the last
current/press points; it does not manufacture a zero-pressure sample.

`unsigned long pressDuration()` is zero in IDLE and otherwise unsigned
`millis() - pressStartTime`, including RELEASED, as in v2. Duration is not frozen
on release. Unsigned wrap is supported for an elapsed interval shorter than one
full timer period. The host rollover test deliberately uses `ULONG_MAX`, not a
32-bit constant, because host unsigned long can be wider than Arduino's.

### Intentional differences from the v2 reference

- `begin()` and `setCalibration()` return `bool` instead of `void`.
- `isReady()` is added; startup is guarded/retryable and hardware identity is
  noncopyable.
- Invalid calibrations are rejected, coordinates clamp, z is median-filtered,
  effective orientation changes/accepted calibrations reset gestures, and a new
  down immediately after RELEASED is not dropped.

No broader gestures, multitouch event model, or ownership redesign is introduced.

## Commands and evidence

From the repository root:

```sh
python3 tests/host/run.py touch_mapper
python3 tests/host/run.py
```

The existing automatic runner discovers `test_touch_mapper.cpp` and compiles the
actual `TouchMapper.cpp`, `Point.cpp`, and remaining production core, with only
existing hardware boundaries stubbed. No runner or shared stub edits are needed.
ASan, UBSan and leak detection are enabled by that runner.

Strict incremental RED -> GREEN evidence was observed for these vertical slices:

1. Startup/lifetime API: missing TouchMapper header -> startup tests PASS.
2. Gesture/duration: PRESSED assertion failure -> transitions and rollover PASS.
3. Calibration validation: missing setCalibration -> rejection/reset tests PASS.
4. Mapping: coordinate assertion failure -> signed spans/swap/clamping PASS.
5. Rotation: missing setOrientation -> all rotation corners/reset tests PASS.
6. Filtering: three-read/median assertion failure -> x/y/z medians, bypass and
   initialized pressure tests PASS.

Final full host invocation passed all discovered suites, including mapper, touch
Canvas integration, legacy lifecycle/startup, allocation failures and rendering.
Host tests do not prove SPI/I2C behavior, physical touch calibration accuracy,
ESP8266 memory sizes, or overday heap stability. A clean MCU compile and physical
touch verification remain separate integration gates owned by the coordinator.
