# Chart projection and history storage

## Public contract

All existing constructor calls retain `HistoryStorage::NewestFirst`: `values[0]`
is newest and every row is contiguous newest-first. Direct public writes still
work in that default mode. Capacity arguments and defaults are unchanged.

Opt in explicitly, with the policy **after** all existing constructor arguments:

```cpp
Trend t(180, 140, 0, 50, 100, 32, HistoryStorage::Circular);
MultiTrend m(180, 140, 0, 50, 100, 3, 32, 3, HistoryStorage::Circular);
```

Circular `values` is the physical ring, **not** a layout-compatible newest-first
array. Read `t.sample(logicalIndex)` or `m.sample(series, logicalIndex)` instead;
index zero is newest, invalid indices return zero. `latest`, extrema, autofit,
markers, legacy immediate drawing, committed drawing and OOM direct rendering
all use the correct logical history. Extrema scan the entire physical history,
which is equivalent without reordering. Every series has an independent head.
`clear` and `init` reset heads. There is no mirror buffer or pre-draw history copy.
Insertion writes one sample regardless of capacity; default insertion still
shifts history. The policy is fixed at construction.

As before, changing allocation dimensions (`maxValues`/`maxSeries`) requires
`init()` before accessing history; enlarging these public allocation dimensions
without reinitializing is not a supported buffer-resizing operation.

## Projection representation

The cache retains the existing shared-X/per-series-Y/color/marker layout using
16-bit slots. Coordinates are signed int16, colors unsigned uint16. Every
projected coordinate, latest marker coordinate, series color and selected marker
color is checked before narrowing. Anything outside those exact bounds selects
native `int` storage for the entire frame. Thus public offscreen positions and
non-565 int colors are not silently truncated for comparison. Transport still
uses the existing TFT API. Previously committed geometry is erased using its
own format before a format change. Failed growth retains ownership and uses the
existing full/direct-render fallback; the next update retries. Capacity is a
byte high-water mark: returning from wide to compact does not shrink an existing
allocation. `init` releases it.

## Reproduce

```sh
python3 tests/host/run.py chart_storage charts render_charts
python3 tests/rasterized/run_chart_storage.py
python3 tests/rasterized/run_xaxis.py
python3 tests/rasterized/run_multitrend_capacity.py
```

The host runner enables existing `HOST_HEAP_MEASURE`/free/new wrappers for
`chart_storage`, just as for `multitrend_capacity`. CI invokes the isolated raster
runner, which uses installed real Adafruit_GFX,
real GUI_TFT.cpp line rendering, and only the existing panel/bus stubs. It prints
source hashes. Both runners enable ASan/UBSan.

RED/GREEN evidence captured during implementation:

1. Production cache measurement asserted 274 bytes and failed with 548 before
   compact storage; passed with 274 after it.
2. Logical history tests failed compilation for the missing policy/sample API;
   passed after circular insertion/accessors were implemented.
3. Real-GFX circular-vs-default incremental comparison failed at its first
   differing history frame before renderer adaptation, then passed afterward.

The host suite checks actual changed physical samples (one for circular vs N for
legacy), wrap at capacities 1/2/16/32/255, independent series histories, invalid
accessors, lifecycle/OOM, and requested/retained allocations for both policies,
series counts 1–4 and capacities 16/32/255. Raster tests compare all four rotations,
first/incremental/fresh frames, seeded asynchronous series updates, windows,
square waves, axes, gridlines, autofit, thresholds/severity markers, clear/init,
immediate draw/erase, malformed windows/empty geometry, initial and growing-cache
OOM, offscreen wide coordinates, and negative/large int colors.

## Measured host bytes (requested, excluding allocator overhead)

On this host `sizeof(int)==4`. The unmodified base 295748c was also built in a
scratch archive and its existing allocation tests executed, not simulated.
For S=3, N=32, exact capacity 3:

| Item | Base | New |
| --- | ---: | ---: |
| Constructor retained heap | 123 | 123 |
| First projected cache | 548 | 274 |
| Total retained after first render | 671 | 397 |
| Additional steady update allocations | 0 | 0 |

Both history policies use identical heap allocations. Wide coordinates request
548 cache bytes in the same configuration. Cache slots cost
`(N*(S+1)+3*S)*2` bytes in compact mode, versus native-int size in wide mode.

Object-size tradeoff on this 64-bit host: TrendBase 376→376, Trend 392→392,
MultiTrend 408→416 bytes. The mode and single head fit existing Trend padding;
MultiTrend adds a bounded `MAX_MULTITREND_SERIES` byte head array plus a byte
policy, with alignment yielding eight additional object bytes here. It avoids
another allocation, preserves capacity behavior, and supports unsynchronized
series. Embedded ABI sizes can differ; no 32-bit board measurement is claimed.

The pre-existing host capacity regression now checks 16-bit cache slots for its
in-range fixtures; its constructor/capacity-savings assertions remain unchanged.
The existing raster capacity test is unchanged and passes. Clean exact-library
ESP8266 builds passed for the full compatibility matrix, including a new combined
opt-in fixture, the dashboard, and the original repository demo. No firmware was
uploaded, and no physical speedup or MCU free-heap measurement is claimed.
