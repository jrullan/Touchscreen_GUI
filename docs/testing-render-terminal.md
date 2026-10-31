# Terminal selective dirty-line rendering

## Contract and scope

- `update()` compares retained bytes, effective row colors, measured glyph bounds,
  public style/geometry, and Canvas/driver identity against the last committed draw.
  An unchanged update issues no panel drawing calls. Public buffer edits remain
  supported when NUL-terminated within their original capacity.
- Ordinary writes clear and redraw only changed rows. Empty unchanged rows and
  equal-text/equal-color software shifts are skipped. Frame drawing is reserved
  for initialization, style/geometry invalidation, or explicit restoration.
- `show()` and the new `forceUpdate()` restore the complete widget even when the
  snapshot matches, for screens/overlays that overwrite the panel. `forceUpdate()`
  respects `visible`; `show()` sets it true. Hidden writes keep their text and
  request complete restoration on show, or on update after becoming visible.
- Both software scroll directions retain their existing physical buffer and color
  shifting policy. `scroll()`, `scrollUp()`, and `scrollDown()` mutate storage;
  `update()` commits it. Printing still commits automatically, as does `clear()`.
- The original bounded print/`%d` formatting, owned buffers, constructor rollback,
  and destructor ownership remain intact. Allocation geometry (`lines` and
  `maxCharacters`) stays read-only after construction. Changing inherited widget
  size/font affects rendering, not allocated text capacity. There is no Terminal
  `init()`/resize-allocation API; repeated initialization coverage means repeated
  construction/destruction and repeated show/style changes.
- Rendering uses stack-only `Guitft::ScopedWrite` guards after drawability checks.
  Each row erase/text repaint is one scope; allocation, text preparation, metrics,
  comparisons, and cache copies are outside it. Old-footprint erases, widget
  background, and chrome have separate bounded display-only scopes.
- No hardware-scroll commands, controller-global scroll state, ring storage,
  text growth, or unrelated production files are introduced.

For classic-font text that fits its row, dirty updates use one row fill and one
nonempty text draw. Backend wrapping, embedded newlines, overlapping glyphs, or
font/geometry changes that put text outside its nominal row require a complete
composition when something changes. Actual old glyph bounds are retained and
cleared, including overflow outside the widget. This preserves legacy rendering
rather than silently truncating retained text after a font/size change. Such
fallbacks can redraw the frame; unchanged overflow content still draws nothing.
On a driver without `getTextBounds`, cleanup uses fixed-cell estimates instead;
wrapping/custom-font cleanup requires the bounds-capable backend.

The optional exact-text snapshot is independent of the owned line buffers.
Snapshot allocation failure causes a safe complete repaint and a retry on the
next visible update; it never drops text or previously committed erase bounds.
Persistent snapshot OOM therefore cannot promise zero-operation unchanged calls.
Line-buffer allocation failure retains the existing empty/inert terminal behavior.
Switching Canvas/driver paints the new target without dereferencing or erasing
through the former target. The previous screen's lifecycle owns its restoration.
As with other widgets, restoration outside old widget bounds uses Canvas background,
not reconstruction of arbitrary widgets underneath it.

Inherited `Widget::hide()` clears assigned widget bounds, not retained overflow
glyphs or a deferred previous position. Hide before moving; use `show()` for full
restoration. Arbitrary external changes to `visible` followed by `update()` do not
replace the explicit restoration contract.

## Reproduce

From `/home/jrullan/dev/Touchscreen_GUI-v2-ports`:

```sh
python3 tests/host/run.py render_terminal
python3 tests/host/run.py
```

The existing runner automatically discovers the new host test. No shared runner
or hardware stub was edited. Both commands compile actual production translation
units with ASan, UBSan, and leak detection. No board connection is required.

The same test file also contains an isolated real-GFX suite selected with
`TERMINAL_REAL_GFX`. The following standalone builder runs that suite by default.
It can also replay the benchmark against the original Terminal without changing
working-tree files: set `TERMINAL_BASELINE=c883292`. Set `TERMINAL_HOST=1` for the
operation-recorder benchmark instead of real GFX. The baseline run deliberately
selects benchmark-only code, not the new behavioral assertions.

```sh
# Optional: export ADAFRUIT_GFX_DIR=/path/to/Adafruit_GFX_Library
# Optional: export TERMINAL_BASELINE=c883292
# Optional: export TERMINAL_HOST=1
# Optional: export TERMINAL_TRANSACTIONS=1  # real-GFX bus boundary assertions
python3 - <<'PY'
from pathlib import Path
import os, subprocess
root = Path.cwd()
gfx = Path(os.environ.get('ADAFRUIT_GFX_DIR', str(Path.home()/'Arduino/libraries/Adafruit_GFX_Library')))
build = Path(os.environ['TMPDIR'])/'terminal-render-replay'
build.mkdir(parents=True, exist_ok=True)
host = os.environ.get('TERMINAL_HOST') == '1'
ref = os.environ.get('TERMINAL_BASELINE')
exclude = {'Adafruit_FT6206.cpp','Adafruit_STMPE610.cpp','Seeedtouch.cpp','XPT2046_Touchscreen.cpp'}
flags = ['-DARDUINO=10819','-DESP8266']
includes = ['-I'+str(root/('tests/host/stubs' if host else 'tests/rasterized/stubs'))]
if ref:
    old = build/'original'
    old.mkdir(exist_ok=True)
    for name in ('Terminal.h','Terminal.cpp'):
        data = subprocess.check_output(['git','show',ref+':'+name], cwd=root)
        (old/name).write_bytes(data)
    includes += ['-I'+str(old)]
    exclude.add('Terminal.cpp')
    flags += ['-DTERMINAL_BENCH_ONLY']
extra = [str(old/'Terminal.cpp')] if ref else []
if not host:
    for name in ('Adafruit_GFX.cpp','Adafruit_GFX.h','glcdfont.c','gfxfont.h'):
        if not (gfx/name).is_file():
            raise SystemExit('SETUP REQUIRED: install Adafruit GFX or set ADAFRUIT_GFX_DIR')
    includes += ['-I'+str(gfx)]
    flags += ['-DTERMINAL_REAL_GFX']
    if os.environ.get('TERMINAL_TRANSACTIONS') == '1':
        includes.insert(0, '-I'+str(root/'tests/transactions/stubs'))
        flags += ['-DTERMINAL_TRANSACTIONS']
    extra += [str(gfx/'Adafruit_GFX.cpp')]
else:
    extra += [str(root/'tests/host/allocation_hooks.cpp')]
    flags += ['-Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc']
includes += ['-I'+str(root)]
sources = [str(p) for p in sorted(root.glob('*.cpp')) if p.name not in exclude]
binary = build/'terminal'
cmd = ['g++','-std=c++17','-g','-O1','-fno-omit-frame-pointer','-fno-pie','-no-pie',
       '-fsanitize=address,undefined','-Wno-write-strings','-Wno-conversion-null','-Wno-return-type',
       *flags,*includes,str(root/'tests/host/test_render_terminal.cpp'),*extra,*sources,'-o',str(binary)]
subprocess.run(cmd, check=True)
subprocess.run([str(binary)], check=True, env=dict(os.environ,
    ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
    UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
PY
```

Do not add `tests/host/stubs` to a real-GFX build: it would shadow the real library.
The raster test uses the installed Adafruit_GFX.cpp and its included glcdfont.c,
with only Arduino/bus/panel substituted. A counting memory panel records actual
in-screen pixel writes, including glyph rasterization, background fills, and chrome.
These are host workloads, **not SPI timings or measurements on physical hardware**.

## Actual verification and TDD evidence

Incremental RED/GREEN steps were executed before each implementation slice:

1. Unchanged update initially failed: 500 text operations for 100 empty updates.
2. First append then failed: no changed-row drawing after initial suppression.
3. Public style mutation then failed: equal retained bytes suppressed style work.
4. `forceUpdate()` initially failed to compile because the API did not exist.
5. Snapshot-OOM recovery failed the subsequent unchanged-update assertion.
6. Real-GFX cleanup failed with 522 mismatching pixels after growing the font and
   clearing text extending below the widget; saved glyph bounds fixed it.
7. Hidden write followed by visible update failed to restore the erased frame;
   deferred complete restoration fixed it.
8. After integrating the shared SPI nesting API, the real-GFX transaction-boundary
   test first reported **6 begin/end pairs** for one row fill plus `alpha` draw.
   Row-bounded ScopedWrite reduced this to **1/1**, with identical pixels. The same
   test verifies zero transactions on unchanged/hidden updates and no writes while
   the mock panel is deselected. Run the builder with `TERMINAL_TRANSACTIONS=1`.
   The shared `python3 tests/transactions/run.py` tests the driver nesting itself.

The completed host suite passes unchanged/changed operations, pre-capacity appends,
software shifting in both directions, row shrink/empty cleanup, direct color/buffer
edits, keepColors behavior, deferred scroll, repeated equal-content shifts, hidden
writes, show/force restoration, all public style/geometry invalidations, target
switching, constructor failure at every row allocation, repeated snapshot OOM and
retry after moving, and repeated construction/show without reallocating text.
Existing `text` tests additionally verify `%d`, INT_MIN/INT_MAX, null strings,
truncation, tiny capacity, maximum row count, and allocation rollback.

The real-GFX suite compares incremental pixels to a clean composition after both
scroll directions, shrink/empty, color policy changes, font growth, and clear. It
also exercises overlapping multiline input, wrapping at the screen edge, movement
of outlying glyphs, and a later backend wrap/font change. Actual wrap witness:
**272 red glyph pixels rendered; zero stale red pixels after clear**.

Actual full host runner result: every discovered test passed under ASan/UBSan/leak
detection. Actual standalone real-GFX result: `rasterized Terminal: PASS`.
Owned Terminal production files pass `git diff --check`.

## Before / after workloads

Baseline: original `Terminal.h`/`.cpp` from `c883292`, compiled from scratch copies,
with all other sources and test workload identical to the modified version.
Terminal: 120x70, size 1, five rows, 17 retained characters per row, border 1,
position (20,30). `full_shift_up` first fills all rows with distinct numbered text
and colors so **every row actually changes**. `clear` follows that shifted frame.

Host operation recorder (operations include text calls and rectangle primitives;
filled pixels are summed positive `fillRect` areas, not glyphs):

| Workload | Before ops | After ops | Before filled pixels | After filled pixels |
|---|---:|---:|---:|---:|
| 100 unchanged empty updates | 500 | 0 | 0 | 0 |
| First upward append | 5 | 2 | 0 | 1,416 |
| Second upward append | 5 | 2 | 0 | 1,416 |
| 100 unchanged updates with two lines | 500 | 0 | 0 | 0 |
| Full upward software shift | 6 | 10 | 8,024 | 7,080 |
| Clear full contents | 1 | 5 | 8,024 | 7,080 |
| First downward append | 6 | 2 | 8,024 | 1,416 |

All measured steady workloads above have zero frame redraws, before and after.
The original printed every allocated row, including empty rows; its upward append
path did not erase old row text. The new append fills are intentional correctness
work. Full software scrolling and clear use more calls but fill less area: this
slice optimizes changed-row transfer, not number of primitive calls at any cost.
SPI guard integration leaves these operation/filled-pixel workloads unchanged.

Actual in-screen pixel writes through the real GFX rasterizer:

| Workload | Before | After |
|---|---:|---:|
| 100 unchanged empty updates | 0 | 0 |
| First upward append | 62 | 1,478 |
| Second upward append | 116 | 1,470 |
| 100 unchanged updates with two lines | 11,600 | 0 |
| Full upward software shift | 8,137 | 7,193 |
| Clear full contents | 8,024 | 7,080 |
| First downward append | 8,086 | 1,478 |

Measured real-library source hashes:

- Adafruit_GFX.cpp SHA-256: `5047f01e8e6dd3029c73cb9447a915fc06675d2e1cc9ebf6a6c1126f556fcecd`
- glcdfont.c SHA-256: `bd6d60ecb9cc350a83f209d7c62de2fe5f6ce3402d5598cde2988b5b44e83e02`

## Memory and limitations

On this x86-64 host ABI, `sizeof(Terminal)` is **176 -> 352 bytes** (176 added),
including fixed row bounds, effective colors, committed style/target geometry,
cache pointer, and validity flags. An optional single allocation adds exactly
`lines * (maxCharacters + 1)` payload bytes: **90 additional heap bytes** for the
benchmark. Original five owned line buffers remain separately allocated and
unchanged. Allocator metadata is excluded. Empty/inert terminals do not allocate
this cache. Target MCU object size/stack use has not been measured here; do not
substitute the host ABI figures for an embedded memory budget.

Updates still scan bounded retained rows and query text bounds; zero panel work
is not zero CPU work. No hashing is used, so text equality is collision-free.
Allocation failure, damaged overlays, changed geometry/style, or overlapping glyphs
can legitimately require a full composition. No hardware scrolling, physical TFT
validation, board flash, commit, or push was performed.
