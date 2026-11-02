# Terminal scroll storage and host evidence

## Compatibility and ownership

The existing four-argument constructor is retained and delegates to
`ScrollStorage::StableRows`. This remains the default: `linesBuffer[row]` keeps
its allocation address for the Terminal's lifetime, including after scrolling,
printing and clearing. No existing caller must opt into a behavior change.

Applications that do not borrow a fixed logical row address can explicitly use:

```cpp
Terminal log(120, 70, TERMINAL_SCROLL_UP, 1,
             Terminal::ScrollStorage::RotateRows);
```

`RotateRows` changes **only the row-address contract**: row pointers rotate, and
borrowed addresses follow an allocation rather than a logical row. Reacquire
`linesBuffer[row]` after any print/scroll before editing a logical row. Pointers
remain owned by Terminal and are never freed by scrolling. Do not free, rebind,
or alias entries in either policy. Direct writes must remain NUL-terminated
within `maxCharacters`. As before, copying an owning Terminal is unsupported.

Logical row order, colors, insertion direction, partial upward fill,
`linesIndex`, truncation, `%d` formatting, empty/null strings, embedded newlines,
clear, public explicit `scrollUp()`/`scrollDown()`, and bytes after the first NUL
are preserved. Explicit scroll duplicates the exposed edge instead of clearing
it. For that reason rotation still copies **one full row**, including its
otherwise unused tail, into the recycled allocation. It does not copy every
retained row. Colors move alongside pointers; the duplicated edge retains its
old color. Zero/one-row scrolls perform no text copy.

There is no additional text buffer, pointer table, allocator call during scroll,
or cache rotation. The existing logical-row renderer snapshots compare contents
and effective colors, so changing storage addresses alone does not make a frame
dirty. Construction retains the existing atomic line-allocation failure policy
(all rows released, `lines == 0`, inert widget). Snapshot allocation remains
optional and retryable. Destruction frees each owned allocation once, regardless
of its current row position.

`lines` and `maxCharacters` remain constructor-time allocation geometry; do not
change them. Existing `setSize()`, position and font edits affect drawing but do
not resize text capacity. Construct a new Terminal to change capacity. The
raster tests exercise geometry edits without changing allocation geometry.

## Strict RED → GREEN

Before production edits, the new host regression was compiled against actual
base `295748cf6e0a45d08dda2e07ce7a88fd4f56c108` sources. Its opt-in factory has a
SFINAE fallback to the legacy constructor solely to permit an executable RED on
the old API. The test failed at:

```
assert(t->linesBuffer[1] == old &&
       "opt-in must rotate storage instead of copying every row")
```

The isolated instrumented baseline was also run before production edits. It
recorded 800 text-copy calls / 14,400 bytes per 200 mixed-direction scrolls,
532 retained requested bytes, zero scroll allocations, `sizeof(Terminal) == 352`.
The factory now chooses the real opt-in constructor. Its rotation assertion,
content/color comparisons and operation counts all pass.

## Reproduce

From the repository root:

```sh
python3 tests/host/run.py terminal_scroll render_terminal text
python3 tests/rasterized/run_terminal_scroll.py --host
python3 tests/rasterized/run_terminal_scroll.py
python3 tests/rasterized/run_widgets.py
```

All runners compile actual production sources with ASan/UBSan and leak detection.
The isolated runner does not modify shared runners or hardware stubs. The host
measurement uses linker wrappers for real `memmove`/`memcpy`, with compiler
builtins disabled, plus existing allocation hooks. No simulated operation counts
are substituted. Run with `--host --baseline` to select only default storage for
the measurement (also usable with the baseline source/API).

The raster runner compiles the installed `Adafruit_GFX.cpp`, actual classic
`glcdfont.c`, and FreeSans9pt7b through a memory panel. It prints source SHA-256
identities; missing GFX files produce `SETUP REQUIRED`, not a false PASS. Override
`ADAFRUIT_GFX_DIR` if necessary. No shared fake Adafruit_GFX include is used.

## Measured results (64-bit Linux host)

For a 120×70 Terminal: five rows, 17 characters plus NUL per allocation.

| Storage-only workload | Default / base | RotateRows |
|---|---:|---:|
| 100 downward + 100 upward scrolls: copy calls | 800 | 200 |
| Same workload: text bytes copied | 14,400 | 3,600 |
| 200 downward appends: copy calls | 800 | 200 |
| Same append workload: text bytes copied | 14,400 | 3,600 |
| Scroll/append allocations | 0 | 0 |
| Constructor allocations, including owning object | 7 | 7 |
| Retained requested bytes, including owning object | 532 | 532 |
| `sizeof(Terminal)` | 352 | 352 |
| Retained requested bytes after destruction | 0 | 0 |

The baseline revision has the same 532-byte retained payload and 352-byte object.
The new enum fits existing host padding; MCU layout may differ. The 532 bytes
are the 352-byte owning object, 90 bytes of rows and the pre-existing 90-byte
snapshot. Allocator metadata is excluded. These are **host operation counts and
requested payload**, not device RAM totals or physical timing/SPI claims.
Storage measurements deliberately have no Canvas, excluding unchanged renderer
snapshot copies and all graphics work. Pointer/color assignment cost remains
linear in row count. At two rows the copy count is unchanged; at zero/one rows
both policies already copy nothing.

## Verified coverage

- Host real-source behavior oracle: both directions; zero, one, two and capped
  ten-row geometries; partial fill, repeated appends, explicit scrolls opposite
  the configured direction, direction mutation, clear/refill, null/empty input,
  embedded newlines, truncation, integer formatting, all capacity bytes, colors,
  stable default row addresses and distinct opt-in allocations.
- OOM injection at each of five row allocations and snapshot construction, for
  both directions; ASan/UBSan/leak checks include destruction after rotation.
- **652 full-panel pixel comparisons** to the default legacy policy: 24
  combinations of two directions × sizes 1/2/3 × classic/FreeSans × wrap on/off,
  plus two repeated snapshot-OOM scenarios. Actual glyph witnesses are required.
  Includes partial/full appends, both explicit scrolls, public row edits,
  `keepColors`, embedded newlines, hidden appends/show, movement, drawing resize,
  font-size mutation, clear, force repaint, retry after cache OOM, and unchanged
  update suppression (zero pixel writes and transactions).
- Existing `render_terminal`, `text`, and `run_widgets.py` tests pass, including
  real-GFX clean-redraw equivalence, stale-glyph cleanup and scoped transactions.

No hardware accessed, MCU timing claimed, commits made, or changes pushed.
