# STMPE610 polling adapter

## API and startup

`TouchDriver_STMPE610` owns the bundled `Adafruit_STMPE610` by value. Include
`TouchDriver_STMPE610.h` explicitly; no legacy Canvas changes are needed.

```cpp
TouchDriver_STMPE610 touch;           // hardware SPI on STMPE_CS
TouchDriver_STMPE610 otherTouch(7);   // hardware SPI on caller-selected CS
```

The constructor is explicit, with signature
`TouchDriver_STMPE610(uint8_t cs = STMPE_CS, uint8_t beginAddress = STMPE_ADDR)`.
The second argument is forwarded unchanged to the bundled
`begin(uint8_t i2caddr = STMPE_ADDR)`. It is NOT a threshold, SPI mode, IRQ pin,
or transport selector. The bundled one-pin constructor selects hardware SPI;
its no-argument constructor selects I2C. This adapter always uses the one-pin
constructor, even when the adapter itself is default-constructed. The bundled
SPI startup ignores the address argument. I2C and software SPI are not exposed.

The bundled implementation stores pins in `int8_t`, with -1 selecting I2C.
This adapter rejects CS values above 127 at `begin()` before hardware startup;
in particular, 255 must not accidentally request I2C. Validity of a physical
GPIO within 0..127 remains board-specific. The bundled `STMPE_CS` macro is only
defined for its supported platform families.

Each unsuccessful `begin()` makes one controller startup attempt. Callers may
retry later; success is retained and subsequent calls do not reinitialize the
controller. There is no adapter allocation or busy-loop. The bundled startup
still performs its own fixed reset delay, register setup, and SPI mode fallback;
this is not an asynchronous-startup promise. Before success (including after a
failed attempt), touch queries return false/count zero and `readRaw()` sets all
outputs to zero without consulting hardware. The driver is noncopyable and must
outlive any mapper borrowing it. There is no new IRQ registration or teardown.

## FIFO semantics: intentional difference from bundled getPoint

Inspect the bundled source, not upstream Adafruit API assumptions:

- `touched()` returns a nonzero touch-status bit, not a contact count.
- `getPoint()` drains the **entire** FIFO, returns the last record, and leaves its
  local coordinates uninitialized if entered with an empty FIFO.
- `bufferEmpty()` checks FIFO status; `readData()` reads one FIFO XYZ record.

The adapter therefore does not call `getPoint()`. A successful `readRaw()` pops
exactly one oldest queued record using `readData()` after an empty check. An
empty FIFO returns zero XYZ without a read. Coordinates (12-bit XY) and pressure
(8-bit Z, including 0 and 255) are passed through without calibration, threshold,
swap, or rotation. Direct raw reads may consume queued records independently of
current contact; `touched()` and inherited `touchCount()` require both active
contact and a nonempty FIFO, do not pop records, and report at most one contact.
This prevents the mapper from treating an empty FIFO under contact as a sample.

`needsFiltering()` returns false: three median reads would consume three distinct
queued events rather than repeated measurements of one event. Each active
mapper poll consumes just one record in FIFO order. No software filter or
latest-only drain is invented. The bundled setup already requests controller
four-sample acquisition. Polling does not acknowledge/clear interrupt status or
flush a backlog; no IRQ behavior is promised. With no active contact, mapper polls
do not drain queued records. FIFO backlog/overflow and touch-status behavior
need real-device validation, especially with slow polling or rapid re-presses.

## Integrated nominal calibration default

`TouchCal::STMPE610_DEFAULT` is integrated in `TouchCalibration.h`:

```cpp
{3775, 240, 240, 3700, false}
```

These are `{STMPE_MAXX, STMPE_MINX, STMPE_MINY, STMPE_MAXY, false}` using the
bundled experimental endpoints. Descending X incorporates the legacy portrait
X flip; Y remains ascending and axes are not swapped. The mapper intentionally
maps endpoints to last valid pixels 239/319 rather than legacy 240/320. This is a
legacy-derived starting point, not measured calibration. The integrated preset
adds `TouchCalibration.h` constants without changing legacy mapping.

## Host regression and TDD evidence

From the repository root:

```sh
python3 tests/host/run.py touch_stmpe610
python3 tests/host/run.py
```

The local class seam predefines the existing bundled include guard, replacing
only the hardware boundary. It asserts that `getPoint()` must never be called.
Actual production adapter and mapper code run through the unchanged host runner
with ASan, UBSan, and leak detection. No shared stubs or runner edits are needed.

Strict vertical RED/GREEN cycles used the targeted command above:

1. Startup test: RED missing `TouchDriver_STMPE610.h`; GREEN startup, exact default
   and custom CS/address forwarding, retry, success idempotence, pre-ready zero
   outputs, noncopyability, explicit construction, and value ownership.
2. Invalid-CS test: RED `invalidCs` assertion (startup accepted 128/255); GREEN
   rejection before any controller begin.
3. Contact test: RED `contact` call-count assertion; GREEN status normalization,
   empty-FIFO rejection, count zero/one, and non-consuming queries.
4. Raw FIFO test: RED `fifoRaw` empty-check assertion; GREEN safe empty handling,
   exact FIFO order and unchanged pressure including extrema.
5. Mapper test: RED `mapperSamplesOnce` read-count assertion (inherited median
   consumed three records); GREEN one record per active poll and release/idle
   transitions when no usable sample remains.

Final targeted run passed. Full host suite passed all discovered targets,
including `touch_stmpe610`, `touch_mapper`, `touch_canvas`, and `touch_ft6206`,
without sanitizer errors.

## Validation boundary

Host results establish adapter lifecycle, arguments and FIFO integration only.
No STMPE610 hardware, SPI electrical behavior, pressure quality, calibration,
FIFO timing/overflow, or physical touch validation was performed. No Arduino
build, flashing, serial access, commit, or push was performed for this task.
The connected XPT2046 device cannot validate STMPE610; the running panelboard
was not accessed.
