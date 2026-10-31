# Phase 1: chart and indicator host safety

Run from the repository root:

```sh
python3 tests/host/run.py charts
```

`tests/host/test_charts.cpp` has one main and exercises the actual Indicator,
TrendBase, Trend, MultiTrend, Gauge, and Dial implementations. The shared runner
links actual core sources, substitutes the hardware-facing Arduino/display
headers, and enables AddressSanitizer, UndefinedBehaviorSanitizer, and leak
detection. Shared allocation hooks fail real C allocations once at a selected
allocation index; the tests do not replace production history buffers with mocks.

## Test-first execution record

Each production change followed an observed failing test, then the chart suite
was rerun to green. Initial harness creation overlapped this work: the first
numeric-formatting red used actual Indicator.cpp with a temporary minimal
bootstrap harness outside the repository; subsequent reds/greens used the shared
runner above. The initial shared-runner attempts failed because run.py did not
yet exist; those infrastructure errors were not counted as behavioral reds.

| Vertical slice | Observed red | Green change/result |
| --- | --- | --- |
| Signed numeric formatting | `setNum(-123)` string assertion failed in bootstrap executable | Bounded snprintf and an int-width-sized buffer; negative, zero, INT_MIN, INT_MAX assertions pass |
| Indicator ownership | LeakSanitizer reported the base label allocation (15 bytes on this host) | Indicator destructor frees its buffer; shared suite passes leak detection |
| Gauge/Dial double allocations | LeakSanitizer reported two lost base allocations, 30 bytes total | Derived constructors no longer allocate a second label buffer |
| Zero Trend capacity | Empty minimum assertion failed because malloc(0) storage was read | No zero-sized allocation/read; min/max return configured bounds and insertion is safe |
| Windows and single-point mapping | Nonzero-window right endpoint assertion failed | Clamped/sorted inclusive windows; index is relative to window minimum; zero span has a deterministic endpoint |
| Equal/reversed scales and negative CV | UBSan division by zero in TrendBase::getYVal | Shared bounded scale mapping handles equal bounds; setLimits orders bounds/clamps setpoint; negative CV clamps to minimum |
| MultiTrend marker capacity/lifecycle | ASan stack-buffer-overflow in markerPrevY initialization with requested capacity 255 | Capacity clamps to MAX_MULTITREND_SERIES; pointers initialized; reinit frees old history/palette; invalid rows return null |
| Unattached drawing | UBSan null Canvas access in TrendBase::drawValuesSeries | Drawing checks Widget::canDraw and label availability; unattached public show/update/draw/erase calls are safe |
| Square-wave rising and falling edges | Vertical operation height assertion failed (negative delta converted to unsigned length) | Signed endpoint ordering and positive inclusive segment lengths; both vertical directions pass |
| Narrow tick spacing | UBSan division by zero in TrendBase::drawXScale modulo | Tick intervals are at least one; later axis mapping shares sample coordinates |
| Label-buffer OOM | ASan null write in Indicator::clear | Null-safe shared clear; Gauge/Dial clear delegates to it |
| History/palette OOM and recovery | UBSan null history load in Trend::update with autofit enabled | Missing/zero history guards in update/autofit; constructor/reinit failure and subsequent recovery pass |
| Autofit below configured minimum | Scale assertion failed when sample 1 produced unsigned minimum underflow | Autofit lower margin saturates at zero; no-series MultiTrend does not change bounds |
| Gauge Y/height above 255 | Border position assertion failed at y=300 | Geometry temporaries use int rather than byte; y=300 and height=400 preserved |
| Tiny geometry/equal-scale drawing | UBSan division by zero in Gauge::drawFill | Safe fill mapping, positive interior/border bounds, Dial scale-safe mapping/radii, chart chrome dimension guards |
| Default construction/reinit | Default Trend capacity/history assertion failed | Initialized defaults and default constructors run init; repeated init/destruction passes leak detection |
| Nonzero-window ticks/time labels | Duration/oldest-tick assertion failed | Axis uses getXVal and window-relative elapsed time; complete 20:00 label and endpoint tick verified |
| Public numSeries increase | ASan palette heap-buffer-overflow in MultiTrend::drawTrend | All rendering loops bound active count by allocated public maxSeries and fixed marker maximum |
| Tiny X mapping/insets | Width-one getXVal assertion failed | Geometry margins/border clamp to available width; collapsed charts map deterministically |
| Gauge constructor limits | Configured high-limit assertion failed | Parameterized constructor initializes thresholds/current value from normalized configured limits |
| Public extreme window fields | UBSan signed overflow subtracting INT_MIN from INT_MAX in getXVal | Coordinate/plot entry points normalize the public window before arithmetic |
| Dial zero tick step | SIGALRM after a two-second watchdog detected an infinite tick loop | Zero tick step falls back to endpoint ticks; suite completes without alarm |

Additional coverage passes without further changes: real label/history/palette
allocation failures; capacity-one histories and markers; reinit after failed
allocation; series-major newest-first history order; invalid series min/max;
hidden widgets (including forceUpdate) produce no operations while accepting
samples; a Canvas with null TFT; INT_MIN/INT_MAX formatting; and tiny widgets
produce no negative recorded widths/heights.

## Behavior and scope

- Public signatures and history layout remain unchanged: Trend newest sample is
  `values[0]`; MultiTrend rows remain series-major, newest-first, stride maxValues.
  Insertion remains the existing linear shift (no ring/performance redesign).
- Zero history capacity is an empty chart. Its min/max fall back to scale bounds;
  insert/latest are safe no-ops/zero. A one-point window maps without division.
- Windows are inclusive, clamped to the configured sample count, and sorted when
  reversed. Equal scales map to the low-value/bottom endpoint rather than divide.
- MultiTrend series capacity cannot exceed its fixed marker array maximum.
  numSeries may select fewer series but cannot make render loops exceed capacity.
- Allocation failure leaves a valid destructible object with null storage.
  Missing label storage disables drawing; missing history disables plotting and
  autofit. Reinit releases old history/palette and may retry history allocations.
- Hidden widgets never draw, including forceUpdate. Data insertion remains usable
  without a display or while hidden.
- Existing public capacity fields are configuration: after changing maxValues or
  maxSeries, call init before using history. Direct external replacement of owned
  buffers or inconsistent capacity mutation without init is not supported.

## Verification and limitations

Final chart runner output is `BUILD charts`, `RUN charts`, `charts: PASS`,
`PASS charts`, exit 0, with no sanitizer/leak findings. A full
`python3 tests/host/run.py` run also passed charts, core, lifecycle, and text.
Earlier runs exposed
unrelated Terminal.cpp NULL-arithmetic warnings while another worker was changing
that file; the final chart build has no such warnings.

These are host safety/operation assertions, not physical TFT/touch validation.
No device was flashed; no commit or push was made. The host path does not compile
the STM32-specific display driver branch or prove embedded 16-bit-int behavior,
though arithmetic and buffer sizing use explicit wide intermediates/int size.
The test's zero-tick watchdog uses POSIX alarm and is intentionally host-only.
