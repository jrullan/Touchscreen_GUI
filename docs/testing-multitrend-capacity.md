# MultiTrend exact-capacity characterization

> Historical measurements below describe the pre-compaction baseline at `295748c`.
> Current in-range projection caches use 16-bit slots, and the regression assertion
> has been updated accordingly. Constructor capacity savings are unchanged; current
> render-cache measurements, circular-history opt-in and object-size tradeoffs are
> documented in [testing-chart-storage.md](testing-chart-storage.md).

Scope: branch `test/multitrend-capacity`, base `05b3b16`. These tests characterize
**existing behavior**, not a newly implemented optimization or a RED/GREEN feature.
No production source or Designer source is changed. Public default remains four
allocated series; history remains newest-first shifting storage, with the existing
`int` projection cache unchanged.

For a fixed active count `S`, use the existing final constructor argument:

```cpp
MultiTrend chart(180, 140, 0, 50, 100, S, N, S);
```

Only do this when spare series are not needed. Default capacity permits writes to
inactive spare rows; exact capacity deliberately does not. Increasing `numSeries`
does not grow storage. Do not increase public `maxSeries` without a corresponding
`init()` (which clears/reallocates history). Comparisons here always use the same
active count and do not claim that spare-capacity behavior is identical.

Read-only inspection of `Touchscreen_GUI_Designer/engine/src/live.cpp:588-592`
confirmed both live construction and emitted code already pass `maxSeries=series`.
There is no additional Designer saving to implement.

## Reproduce

From the repository root:

```sh
python3 tests/host/run.py multitrend_capacity
python3 tests/rasterized/run_multitrend_capacity.py
python3 tests/host/run.py
python3 tests/rasterized/run_xaxis.py
python3 tests/rasterized/run_icon_runs.py
```

The isolated raster runner requires installed Adafruit GFX (default
`~/Arduino/libraries/Adafruit_GFX_Library`, override `ADAFRUIT_GFX_DIR`). Missing
sources fail explicitly as SETUP REQUIRED, not PASS. It prints source SHA256s.
Both runners compile actual GUI translation units under ASan/UBSan with leak
checking; only hardware/Arduino boundaries are replaced. CI runs the new host
measurement executable and the real-GFX capacity equivalence suite. Integration
also validated and freshly generated Designer's `ChartFlags` project into scratch,
confirmed emitted capacities 3/3/2 match active counts, and clean-compiled it for
ESP8266 D1 Mini against this exact library worktree. No firmware was uploaded and
no MCU heap measurement is claimed.

## Measured heap evidence

Existing `allocation_hooks.cpp` C/new/delete wrappers count successful requested
payload bytes and track retained allocations. Constructor measurement includes
Indicator's **15-byte numeric-label buffer**, the `S*N` byte history, and the
`S*sizeof(int)` palette. There is no label-storage optimization. Constructor
allocation count remains **three** for both policies. Stack object size is 408
bytes on this host and is not included in the heap numbers.

Host `sizeof(int)=4`. Every row below is actual observed retained payload bytes;
not allocator usable-size, metadata, fragmentation, physical free RAM, or MCU
measurement. Render/update columns are totals including constructor allocations.
All 24 measured lifetimes retained **0 bytes after destruction**.

| N | S | Constructor default4 / exact | First render default4 / exact | Changed update default4 / exact | Retained saving |
|---:|---:|---:|---:|---:|---:|
|16|1|95 / 35|235 / 175|235 / 175|60|
|16|2|95 / 55|311 / 271|311 / 271|40|
|16|3|95 / 75|387 / 367|387 / 367|20|
|16|4|95 / 95|463 / 463|463 / 463|0|
|32|1|159 / 51|427 / 319|427 / 319|108|
|32|2|159 / 87|567 / 495|567 / 495|72|
|32|3|159 / 123|707 / 671|707 / 671|36|
|32|4|159 / 159|847 / 847|847 / 847|0|
|255|1|1051 / 274|3103 / 2326|3103 / 2326|777|
|255|2|1051 / 533|4135 / 3617|4135 / 3617|518|
|255|3|1051 / 792|5167 / 4908|5167 / 4908|259|
|255|4|1051 / 1051|6199 / 6199|6199 / 6199|0|

The first full-window render adds one cache allocation of
`sizeof(int) * (N*(S+1)+3*S)` under either capacity policy. The subsequent changed
update adds no allocations. Saving is only `(4-S)*(N+sizeof(int))`; the cache
already scales with the **active** count, not capacity. On a target with a
different `sizeof(int)` the palette/cache totals differ; allocator overhead and
board-specific available RAM must be measured separately.

Canvas hardware-seam buffers are created and its operation vector is reserved
before starting measurement. Recorded short labels fit host string SSO. Assertions
on exact allocation counts and retained bytes detect accidental seam allocation
contamination rather than silently calling it widget RAM. C++ new/delete routes
are enabled as in the existing optional-allocation test, which also calibrates
the instrumentation. No synthetic allocation results are supplied.

## Coverage and observed results

- Host matrix: S=1..4, N=16,32,255; repeated insertions beyond history capacity
  independently check every newest-first history byte and min/max values.
- Auto-fit checks full-history extrema even with a narrower visible window;
  `autoFit(false)` preserves current scale. Invalid series S..255 must leave
  all valid history, palette, and currentValue unchanged; getters use fallback.
- Clear preserves palette and zeros all rows; reinit resets history/window.
- Both history and palette OOM are injected individually at construction and
  reinit; drawing/getters/update remain safe and a subsequent init recovers.
- Real-GFX matrix: **48 configurations, 816 full physical-panel frame comparisons**
  across all four rotations. Uses installed `Adafruit_GFX.cpp`/classic glyphs,
  actual `GUI_TFT.cpp`/`Guitft::drawLine`, and the existing rotation-aware
  `icon_stubs` panel boundary. No fake chart/line implementation is substituted.
  Nonblack and changing-frame witnesses prevent a no-op renderer from passing.
- Raster stages cover severity markers, radius off/on, axis/border/threshold/grid
  toggles, square waveform, window/time scale, manual/automatic auto-fit,
  clear/reinit, reducing/restoring active count, hidden mutation/show, forceUpdate.
  Transactions must be balanced after each frame. This proves capacity-policy
  equivalence for this workload, not independent correctness of every chart pixel.
- Full host suite: **27 executables PASS**, including instrumentation calibration,
  chart cache/OOM regressions and the new capacity characterization.
- Existing real-GFX X-axis suite: **361 cases each for Trend and MultiTrend PASS**.
  Existing icon real-GFX suite: **11358 cases PASS**.

No production bug was found by this coverage. New tests passed the existing
implementation; they are explicitly regression/characterization tests, not a
claim of test-driven implementation. A first-run printf format warning in the new
test was corrected before the full-suite run. No commit, push, MCU firmware
change, hardware operation, or Designer edit was performed.
