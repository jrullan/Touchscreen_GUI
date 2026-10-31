# Gauge / Slider retained rendering

## Contract

- `setCV(value, false)` changes assigned state only. `previousValue` retains its
  legacy meaning (previous assignment); it is **not** the rendering baseline.
- `update()` compares against the last completed render on the current Canvas
  and driver. Equal mapped position and effective RGB565 fill color emit no
  display operations, including distinct numeric values that map to one row.
- A stable-style grow/shrink emits one interior strip. A Gauge threshold crossing
  with a different effective color repaints its interior, not its scale/border.
- Public geometry, background, foreground, border, scale and Gauge threshold /
  label changes trigger a complete bounded rebuild. The old committed footprint
  is restored to the current Canvas background before the new frame is drawn.
  Nonpositive dimensions paint nothing and remove any previously committed frame.
- Gauge keeps its existing layout: `w` is the bar width, with the scale alongside
  it; its footprint extends through `x + yScaleWidth + w`. Cleanup also retains
  classic size-one label bounds, including screen-edge wrapping and tiny-height
  label overflow. Normal full rebuilds do not clear the screen.
- `show()` is an unconditional rebuild and makes the control visible.
  `forceUpdate()` rebuilds without making a hidden control visible.
  `invalidate()` requests a rebuild on the next drawable update, without traffic
  at mutation time. Invalidating retains the old footprint for cleanup.
- Concrete `Gauge::hide()` / `Slider::hide()` erase committed bounds, even after
  deferred moves. Hidden updates emit no traffic. A later `show()` rebuilds.
  **Widget::hide is nonvirtual:** calling it through a `Widget*` still executes
  the old base implementation, which cannot erase Gauge's outlying scale/bar or
  an old deferred position. Use the concrete hide method when immediate full
  cleanup is required; `show()` repairs after either path. Widget is unchanged.
- `Gauge::drawFill()` commits the assigned value. `Slider::drawCV(int)` commits
  the requested visual value without changing `currentValue`; the next update
  reconciles the assigned value against that actual rendered baseline.
  Public `drawBorder()` preserves its border-only operation and records a dirty
  union of intermediate footprints so a subsequent update/hide removes them.
- Canvas/driver replacement forces a rebuild on the new target. Old targets are
  not dereferenced or painted. Clean the old target before detaching if desired.
- Slider's handler, hit bounds, mapping, debounce comparison/timestamp, and block
  return convention are unchanged. Its already-committed touch value is no longer
  painted a second time with the raw intermediate touch value.
- Gauge's signed/unsigned setters and safe full-`UINT_MAX` mapping are unchanged.
  Rendering uses the existing guarded scale mapper; empty/reversed scales and
  degenerate borders do not produce negative-sized interior fills.

The driver has no transaction failure acknowledgement: “completed render” means
all synchronous drawing calls returned. Automatic detection of outside drawing,
custom backend font/wrap changes, rotation changes, and overlapping-widget scene
recomposition are not provided. Use show/forceUpdate/invalidate after external
changes. Footprint restoration assumes a uniform Canvas background, not a saved
image of underlying widgets. Gauge label bounds assume the classic 6x8 font and
normal wrapping used by Guitft, not an externally selected custom font.

## Regression execution

```sh
python3 tests/host/run.py render_bars
python3 tests/host/run.py
BAR_BENCH_ONLY=1 python3 tests/host/run.py render_bars
```

These compile actual GUI translation units with ASan/UBSan and leak detection.
The operation recorder rasterizes rectangles/lines but not glyphs. The same new
bar test has a second build mode using the installed **real Adafruit_GFX.cpp and
its glcdfont.c**, with only panel/bus/Arduino boundaries substituted:

```sh
python3 - <<'PY'
from pathlib import Path
import os, subprocess
r = Path.cwd()
g = Path(os.environ.get('ADAFRUIT_GFX_DIR', str(Path.home()/'Arduino/libraries/Adafruit_GFX_Library')))
assert (g/'Adafruit_GFX.cpp').is_file(), 'Install Adafruit GFX first'
b = Path(os.environ['TMPDIR'])/'bars-real-gfx'
excluded = {'Adafruit_FT6206.cpp', 'Adafruit_STMPE610.cpp', 'Seeedtouch.cpp', 'XPT2046_Touchscreen.cpp'}
sources = [str(p) for p in r.glob('*.cpp') if p.name not in excluded]
transaction_flags = (['-DBAR_TRANSACTIONS', '-I'+str(r/'tests/transactions/stubs')]
                     if os.environ.get('BAR_TRANSACTIONS') else [])
cmd = ['g++', '-std=c++17', '-g', '-O1', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie',
       '-fsanitize=address,undefined', '-Wno-write-strings', '-Wno-conversion-null', '-Wno-return-type',
       '-DARDUINO=10819', '-DESP8266', '-DBAR_REAL_GFX', *transaction_flags,
       '-I'+str(r/'tests/rasterized/stubs'), '-I'+str(g), '-I'+str(r),
       str(r/'tests/host/test_render_bars.cpp'), str(g/'Adafruit_GFX.cpp'), *sources, '-o', str(b)]
subprocess.run(cmd, check=True)
subprocess.run([str(b)], check=True, env=dict(os.environ,
    ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
    UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
PY
```

Do not add `tests/host/stubs` to the real-GFX include path: it would shadow GFX.
Set `BAR_TRANSACTIONS=1` in the environment before the command above to also
exercise the parent's instrumented real-GFX transaction backend. This passed for
both controls: six rendered operations produced **6 begins / 6 ends**, zero
writes outside selection, zero transactions for unchanged/hidden/empty-interior
updates, and the Slider callback observed the display deselected. The first
transaction regression failed before outer guards were added; a second RED
caught useless empty-interior transactions and is now GREEN. The standalone
`python3 tests/transactions/run.py` suite also passed.

Outer `Guitft::ScopedWrite` guards cover only actual drawable render/rebuild,
border-only draw, and committed-footprint hide paths. No-op checks run first;
guards are never placed around `checkTouch`, event callbacks, yields, or other
SPI devices. No display primitive is overridden by the bar widgets.

The real-GFX run passed, with **79 white glyph pixels** witnessed in Gauge's
scale area before comparisons. Incremental frames equal a screen-cleared show
across deferred writes, edge wrapping, unsigned wide labels, tiny heights,
moves, resize, style/threshold changes, and empty commits.

The host test also asserts no-op traffic, one-strip growth/shrink, hide after a
deferred move, all repair paths, threshold crossing with equal effective RGB565
colors, out-of-range/degenerate geometry, direct paint reconciliation, accepted
repeat-touch suppression, transient border footprints, and Canvas/driver swaps.
RED failures were observed before the corresponding implementation changes for
no-op suppression, single-strip deferred commits, moved-frame pixel equivalence,
committed-footprint hide, explicit repair APIs, negative-border/empty geometry,
direct-paint reconciliation, and effective-color no-op suppression. The existing
charts suite caught the importance of preserving border-only `drawBorder()`'s
large-coordinate/non-byte geometry contract; it passes after preserving that API.

## Measured operation / pixel traffic

Same deterministic host workloads, initial show excluded. Baseline uses Gauge.h,
Gauge.cpp and Slider.h from **c883292**; final uses the implementation alongside
this test. Baseline mixed-workload numbers were remeasured by compiling copies
of those original files from `git show c883292:<file>` with this exact benchmark.
`BAR_BENCH_ONLY` skips regression assertions, not workload steps.

| Workload (20 commits) | Baseline ops | Final ops | Baseline pixel writes | Final pixel writes |
|---|---:|---:|---:|---:|
| Gauge stable-color changed | 40 | 20 | 83,520 | 16,560 |
| Gauge unchanged after stable | 40 | 0 | 83,520 | 0 |
| Slider stable-color changed | 240 | 20 | 96,160 | 16,560 |
| Slider unchanged after stable | 240 | 0 | 96,160 | 0 |
| Gauge mixed changed | 58 | 120 | 73,192 | 118,358 |
| Gauge unchanged after mixed | 40 | 0 | 59,520 | 0 |
| Slider mixed changed | 240 | 85 | 80,560 | 82,412 |
| Slider unchanged after mixed | 240 | 0 | 70,160 | 0 |

Stable workload: three deferred writes per commit, ending alternately at 60/40,
40x120 controls, border two, fixed thresholds and colors. Mixed workload: three
deferred writes ending at `(frame*37)%101`, a move at frame 4, resize at 8, colors
at 12, scale change at 16, unconditional show at 18, updates otherwise. It includes
threshold crossings and real rebuilds, not just repeated unchanged calls.

The mixed changed workload is **not a universal improvement**: baseline Gauge
updates did not repaint moved/style/scale chrome or erase old footprints, and
Slider did not restore moved footprints either. Correct cleanup/rebuilds write
additional pixels; baseline's lower pixel count is not equivalent output.

Operations are recorder entries (a rectangle records itself plus its four edge
lines), not measured SPI transactions. Pixel writes sum requested positive areas
of fillRect/hline/vline entries, including overdraw and excluding text glyphs;
they are not changed-pixel counts or clipped bus-byte measurements. No hardware
latency, frame-rate, SPI timing, or memory-bandwidth claim is made.

## Memory

Measured x86-64 host `sizeof`: Gauge **112 -> 232 bytes (+120)**; Slider
**136 -> 200 bytes (+64)**. Each stores one fixed-size target/style/position
snapshot; Gauge also retains label/frame bounds. There is no new heap cache,
capacity-dependent allocation, or per-update allocation in production code.
Indicator's existing numeric formatting buffer remains unchanged. Stack-local
snapshot and label-formatting storage are bounded. These are host ABI sizes,
not MCU sizes; recompile the existing sizes sketch for a target-specific ABI
measurement rather than extrapolating host pointer sizes.
