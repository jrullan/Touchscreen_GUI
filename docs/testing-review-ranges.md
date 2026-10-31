# Independent review: unsigned ranges and tiny chart geometry

## Compatibility decisions

- Keep the existing `setCV(int, boolean)`, `setNum(int)`, and
  `Gauge::getYVal(int)` / `TrendBase::getYVal(int)` entry points. Ordinary
  integer literals and promoted byte/short samples still select the signed
  overload without ambiguity. Negative values clamp to the unsigned scale
  minimum; signed labels, including `INT_MIN`, retain their sign.
- Add exact `unsigned int` overloads for those entry points. Configuration,
  current values, previous values, labels and Y-coordinate mapping now preserve
  values through `UINT_MAX`. Both mapping overloads delegate to an `int64_t`
  helper before any narrowing; both current-value overloads share one clamping
  helper. Existing equal-range fallback and `setLimits` reversal normalization
  remain unchanged.
- This is an int/unsigned-int API, not a new arbitrary-width integer API.
  Callers with long/long-long arguments should explicitly select a supported
  type after checking its range; overload addition can make such calls or
  untyped member-function-address expressions ambiguous. Existing typed
  pointers to the signed member functions remain valid.
- Dial needle input is unsigned, matching `currentValue`/`previousValue`.
  Mapping produces a signed angle only after the full unsigned value reaches
  `mapScale`. Current-value text uses the unsigned label buffer and `drawString`
  rather than conversion to the display wrapper's signed `long` argument.
  Existing font size, spacing, colors, and needle angle endpoints are preserved.
- Trend and MultiTrend sample storage and `addValue` remain byte-based. Full
  unsigned configuration does not expand history samples or change the signed
  min/max sample-query APIs. Automatic fitting still derives its range from
  byte samples.
- Y-scale background height uses `chartH()` (therefore no X-scale reservation
  when X labels are disabled), retaining the existing border extension.
  Background rectangles are emitted only for positive width and height.
  This prevents negative dimensions; it is not a general text clipping/layout
  change for labels that cannot fit in a one-pixel widget.
- `Indicator::init()` retries allocation only when its constructor-assigned
  label pointer is null. It does not call derived initialization from the base
  constructor, replace live storage, or allocate on every update. Successful
  retries initialize the whole buffer. Persistent failure remains safe through
  existing null guards, and concrete widget initialization can recover from a
  transient constructor allocation failure immediately.

## Test-first execution evidence

Each slice used `python3 tests/host/run.py review_ranges` before and after its
production change. Observed RED results, in order:

1. `unsigned_indicator_values`: `n.getCV() == UINT_MAX` failed.
2. `unsigned_chart_mapping<Gauge>`: `chart.getYVal(UINT_MAX) == top` failed.
3. `unsigned_dial_display`: the buffer did not contain the current unsigned
   value (`strcmp(d.text(), expected) == 0` failed).
4. Needle input contract: compilation failed at the explicit static assertion
   requiring an unsigned value parameter, rather than a signed int parameter.
5. `tiny_y_scale_geometry`: the recorded width/height nonnegative assertion
   failed for `Trend(40, 1, ...)`, X scale off and Y scale on.
6. `label_allocation_retry`: `init()` left the buffer null after transient OOM.

Every slice subsequently returned `review_ranges: PASS`. The final targeted
run and `python3 tests/host/run.py` both passed with ASan, UBSan and leak
checking enabled by the existing runner. The full run passed: charts, core,
lifecycle, new_oom, review_ranges, review_startup, text, visibility.

## Coverage and limits

The new test compiles real production widgets against the existing hardware
stubs. It checks signed/unsigned current-value clamping and previous-value
tracking, suppression/request of updates, signed formatting, unsigned decimal
labels, full and high-only unsigned scale endpoints and intermediate mapping,
equal and reversed scales, Dial current-value rendering, tiny chart rectangle
dimensions with X labels both enabled and disabled, allocation recovery, and
reuse of a live label allocation.

The existing display stub does not record or rasterize `fillTriangle`.
Consequently, the needle regression uses an explicit compile-time parameter
contract check (test-only private access) plus execution of real Dial show and
update paths; it does not claim a pixel-level needle endpoint assertion. The
host is also LP64, unlike common 32-bit Arduino targets, so checking that Dial
formats through the buffer is deliberate: host `long` would otherwise conceal
the display wrapper's target-specific signed narrowing. No device build,
flash, or physical-display check was performed.
