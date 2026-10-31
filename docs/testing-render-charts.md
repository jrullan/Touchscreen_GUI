# Chart acquire/commit rendering

## Contract

- `Trend::addValue(value, false)` and `MultiTrend::addValue(series, value, false)` change newest-first public history only. `clear()` also changes history only. None emits display operations, even when visible.
- `update()` commits all pending mutations in one frame. Default/immediate `addValue(...)` still calls it. A visually unchanged committed state emits zero operations; raw history changes that project to identical pixels need no repaint.
- A normal data commit erases **all previously committed** polylines, then all committed markers, restores gridlines and thresholds, draws all new polylines and markers, and restores the border last. No ordinary data commit clears the whole widget. Old marker X, Y and radius are retained; erasure uses the exact old radius, not radius+1.
- Geometry, scale, window, chrome, waveform and marker-style changes select a controlled old/new-region redraw. Palette changes can use the normal dynamic commit. An exact 36-int style signature avoids probabilistic hash collisions, including mutations through public fields. Target Canvas/display identity is compared separately, without dereferencing an expired former target.
- `invalidate()` requests a full bounded repaint on the next `update()` and emits no operations itself. `forceUpdate()` explicitly repaints immediately, and `show()` makes visible and repaints even if unchanged. Use these after overlays/external painting; ordinary `update()` cannot detect arbitrary external pixels. Hidden widgets remain non-drawing even for `forceUpdate()` and `autoFit()`; `show()` is the explicit visibility escape hatch.
- `autoFit()` calculates limits before the single commit, not as a second nested rendering pass. The latest-value edge trigger and all-history min/max fitting are retained.
- Legacy `drawTrend()` remains a layer-only drawing helper, not the acquire/commit API; it retains a union of every intermediate widget/marker footprint on the current target and invalidates the next coherent update. The next commit clears this bounded union before repainting. Switching Canvas or display discards former-target cleanup rather than drawing through possibly expired pointers; callers own clearing the former target. `eraseTrend()` erases committed dynamic geometry and invalidates the next update. Use `show()`/`forceUpdate()` for a complete repaint.
- Public histories remain newest-first shift arrays, not rings. Public capacity fields remain allocation metadata: callers must use `init()` after changing allocated capacities; this patch does not make arbitrary larger capacity assignments safe.

## Memory and failure handling

One lazy, grow-only allocation contains shared projected X coordinates, per-series projected Y coordinates, line colors, marker colors and marker Y coordinates. For a visible window of N samples and S active series:

```
cache payload = sizeof(int) * (N * (S + 1) + 3 * S)
```

Capacity is a high-water mark until `init()` or destruction. Coordinates use native `int` rather than narrowing caller geometry to int16. With 32-bit ints: 3 series × 32 samples = **548 bytes**; 2/3/1-series × 60-sample dashboard charts = **744 / 996 / 492 bytes**, or **2,232 bytes** combined. Default maximum 4 series × 255 samples = **5,148 bytes**. No framebuffer, raw-history duplicate, or per-update allocation is introduced after sufficient cache capacity exists.

New object fields before ABI padding are 5 pointers, one `size_t`, 43 ints and 4 bools. The cleanup follow-up adds four union-bound ints and two non-owning target pointers: 32 bytes on this x86-64 host (24 bytes before padding with 32-bit ints/pointers), with no additional heap allocation. MultiTrend removes its previous `MAX_MULTITREND_SERIES * sizeof(int)` marker-Y array; Trend removes an unused update counter. Actual Linux x86-64 host `sizeof` measurements:

| Class | Baseline ad5abc5 | New | Delta |
|---|---:|---:|---:|
| TrendBase | 144 | 376 | +232 |
| Trend | 160 | 392 | +232 |
| MultiTrend | 192 | 408 | +216 |

Thus the benchmark MultiTrend's extra host object + cache payload is **764 bytes**, excluding allocator overhead. Board ABI/object sizes must be measured separately. The local style snapshot uses 36 native ints (144 bytes on 32-bit-int systems); this is not a measured peak-stack claim.

The cache uses one checked `realloc`. Initial/growth failure preserves histories and any owned old allocation, renders a complete bounded full-redraw fallback, records last-rendered bounds (including outlying pucks), and retries allocation on a later commit. It never marks a missing/incomplete cache as committed. While allocations keep failing, zero-traffic unchanged updates are not promised: correctness takes precedence over suppression. A subsequent move or erase also removes an OOM fallback frame. `init()` and destruction release cache storage. Before `init()` discards projected coordinates, old widget bounds are expanded to include all committed marker extents for the next cleanup. Existing history/palette allocation-failure paths remain tested.

## Host benchmark: identical workload, real output

The benchmark is embedded in `tests/host/test_render_charts.cpp` and runs before assertions. It attaches a 180×100 bare chart at (10,15), with border, 25/50/75 gridlines, 3 active series, 32 samples and radius-2 markers. After `show()` (excluded), 20 frames each acquire 4 samples per series with `(frame*7 + series*31 + sample*11) % 101` and call `update()` once. Then 20 unchanged `update()` calls are counted separately.

Run the counter without regression assertions against the baseline:

```sh
CHART_BENCH_ONLY=1 python3 tests/host/run.py render_charts
```

| Workload | Before production changes (ad5abc5) | Final |
|---|---:|---:|
| 20 changed batched frames | 65,722 | 22,160 |
| 20 unchanged commits | 26,540 | 0 |

Changed-workload host operation reduction: **66.28%**. The original output was captured before editing production code; removing only mutation-time erases temporarily measured 22,526 changed / 26,540 unchanged operations, but was **not** accepted because framebuffer equality still failed. The final implementation actually erases old committed geometry and passes framebuffer comparisons.

These are recorded hardware-stub operations, not SPI bytes, address windows or board timings. The real `GUI_TFT.cpp` line slicing executes, so slopes affect the number of logged H/V spans; `rect` also logs its boundary spans. Existing stubs rasterize lines and filled circles but log text without rasterizing glyphs. Equality proves the modeled pixel primitives, not font rasterization or physical flicker. Parent integration owns actual-board timing with the existing Phase01_Baseline sketch; no board timing, flash, or endurance/storage test was performed here.

## Verification and RED → GREEN evidence

Commands:

```sh
python3 tests/host/run.py render_charts
python3 tests/host/run.py
```

The unchanged runner compiles actual library translation units with ASan, UBSan and leak detection, plus existing hardware-boundary stubs. No Display, runner or stub edits are required for these tests.

Observed expected RED assertions before the respective fixes:

1. Visible deferred Trend mutations emitted operations (`ops.empty()` failed); removing mutation-time erases made deferred Trend/MultiTrend mutation and clear pass.
2. Deferred multiple mutations left old pixels (`pixels == clean show` failed); projected committed geometry plus unchanged suppression made this pass.
3. Auto-fit failed clean-redraw equality; calculate-first/single-commit fitting made this pass.
4. Nonzero window start put a marker at window data instead of the newest sample (`newest` failed); independent committed marker Y fixed it.
5. Moving after initial cache OOM failed equality; retaining fallback frame bounds fixed it. Growth-failure and Trend fallback are also exercised.
6. Clearing assigned data before `eraseTrend()` left colored committed pixels; committed erasure fixed it.
7. `eraseTrend()` immediately after cache OOM left colored pixels; bounded fallback erasure fixed it.
8. Legacy layer-only `drawTrend()`, followed by another deferred mutation, left intermediate geometry; invalidating that helper fixed the next coherent commit.

### Cleanup follow-up RED → GREEN

Each confirmed defect was added and run against the then-current production source before its fix:

1. Public radius-12, no-inset MultiTrend at (30,40), `show(); init(); show()` failed clean-framebuffer equality. Retaining old marker extents before cache reset fixed it.
2. Trend moved through x=140 with `drawTrend()` before committing x=50 failed equality. Target-tagged dirty unions fix multiple intermediate layer draws; MultiTrend includes outlying markers in the same union.
3. Width zero after a visible frame failed the all-BLACK framebuffer assertion. A committed empty-state path now erases the prior frame, skips every new dynamic primitive, and suppresses unchanged empty updates. Zero/negative width and height are covered; legacy marker drawing also rejects empty geometry.
4. BLACK canvas / GRAY2 widget with a public outlying marker failed equality after `addValue(0,100)`. Marker erasure keeps widget background inside and restores canvas background in bounded outlying strips (no whole-widget wipe for ordinary data frames).
5. Initial cache OOM with that same outlying marker failed an outside-widget BLACK witness after `eraseTrend()`. Uncached full-bound cleanup now uses canvas background, not widget background; later update repairs the widget.

Additional passing regressions switch Canvas and then the owned display driver while legacy dirty geometry exists, requiring no writes to the former target and clean-frame equality on the new target. All 11 actual-source host suites pass with the unchanged ASan/UBSan runner. The deterministic benchmark remains **22,160 changed / 0 unchanged operations**. No flash, hardware operation, commit, Display edit, stub edit or runner edit was performed for this follow-up.

Final tests cover repeated deferred mutation, direct public-history writes, crossing/coincident lines and markers, shrinking/changing series, 0/100 values, square waveforms, single-point windows, marker/window changes, radius enable/disable, severity colors, palette changes, position/size/scale/insets/chrome changes, explicit and automatic fitting, hidden mutation/show, force/invalidate/show after external paint, OOM initial allocation/growth/retry/erase, and no new steady-state allocations. Operation assertions require one old/new marker pass per commit, all erasure before foreground, and no fillRect during an ordinary data frame. All existing safety suites remain unchanged.
