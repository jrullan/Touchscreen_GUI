# Phase 1 text/composite memory safety

## Scope and command

`python3 tests/host/run.py text` compiles the actual core sources with the shared
Arduino/TFT stubs, AddressSanitizer, UndefinedBehaviorSanitizer, leak detection,
and the shared C-allocation failure hooks. No hardware drivers are linked.
`tests/host/test_text.cpp` contains one main and exercises the real classes.

No performance phases, commit/push, board build, or flashing were performed.

## Observed test-first cycles

Each row was added and run before its corresponding production change, followed
by a successful `python3 tests/host/run.py text` run. Earlier regressions remained
in the executable throughout.

| Slice | Observed RED | GREEN change |
|---|---|---|
| Text null input | UBSan null load in `Text::setText(const char*)` | Bounded owned copy, explicit terminator, null/OOM-safe empty operations |
| Button label ownership and destruction | UBSan null load in `Button::setLabel` | Allocate/copy before replacing, preserve old label on OOM, delete owned Text |
| Display short-source append | ASan stack-buffer-overread in `Display::append(char*)` | Copy only through source NUL or remaining capacity; terminate; clean up Text |
| Unattached button/composite rendering | UBSan null Canvas access in `Button::drawLabel` | Guard drawing, initialize callbacks, guard null touch points, unify Option/Icon label ownership |
| Popup message lifetime | ASan invalid free of caller's stack message | Own message copy; initialize both constructors; button captions remain borrowed literals |
| Numkey append and numeric bounds | ASan short-source stack-buffer-overread in `Numkey::append` | Bounded append, null/OOM guards, bounded `%d` formatting including INT_MIN/MAX |
| Terminal invalid geometry and buffers | UBSan division by zero for font size zero | Normalize font/geometry, bounded formatting and scrolling, complete destructor, atomic OOM initialization |
| Grid final one-based ID | ASan heap-buffer-overflow writing `labels[6]` for a 2x3 grid | Correct pointer-array sizing, reserved zero slot, owned labels, count validation, transactional resizing |
| Text ownership copying | Compile-time assertion failed: Text was shallow-copyable | Delete Text copy construction/assignment; Widgets are noncopyable via the shared base |
| Visibility integration | First attached Popup show failed to draw its message | Set visibility before show drawing; retain pre-hide drawing permission for Popup/Numkey |
| Long Button text/geometry | 200-character Button fit-width assertion failed | Avoid signed-char text length and byte-truncated border height |

The first harness attempts were temporarily blocked because the parent had not
created `run.py`, then by a missing Arduino `Print` declaration. The parent fixed
the shared stub; neither shared stubs nor the runner were edited here.

## Additional regression coverage

- Text capacities 4 and 255, termination/truncation, self-overlap, clear, signed
  parsing and overflow saturation to LONG_MAX.
- Button label replacement, overlapping label input, caller mutation, null
  text/labels, normal and 200-character rendering, hide/show.
- Display full-capacity append, short strings, character append, null append,
  delete, fit, destruction and actual attached TFT text operations.
- OptionButton initialization/rendering/destruction; IconButton null images and
  attached bitmap rendering.
- Popup default construction, stack message ownership, replacement, OOM rollback,
  all three caption modes, first show/hide, and absent callbacks/targets.
- Numkey short append, delete, clear, INT_MIN/INT_MAX/-1/0/1, unattached drawing,
  absent targets and allocation failure.
- Terminal both scroll directions, preserved colors, repeated `%d`, INT_MIN/MAX,
  tail `%`, string clipping, Arduino String input, zero-line geometry, clear,
  rendering, destruction, and failure of each of ten line allocations.
- Grid zero/oversized dimensions, first/last/invalid IDs, a 255x1 boundary grid,
  maintained-state toggling at the last cell, owned labels, resizing and failure
  rollback. C-allocation failures cover both grid arrays and label replacement.
- Real C-allocation failures also exercise Text, Button and Display construction.

## Safety contracts

- **Owned-copy setters:** Text/Button/Display text is copied into fixed-capacity
  storage. Button labels and Popup messages are copied into owned allocations.
  Grid labels copy up to eight characters and add a terminator. Input pointers
  are not retained by these setters. Null means empty/clear (null append is a
  no-op). Allocation-backed replacement retains the previous value on OOM.
- **Public pointer compatibility:** `Text::text`, Button/Display `contents`,
  `Button::label`, grid `labels`/`states`, and Terminal `linesBuffer` remain
  accessible, but callers must not free/delete/rebind them or their owned label
  entries. Direct character edits must fit allocated capacity and preserve NUL
  termination. Getter pointers remain owned by the widget and may be null on OOM.
- `Text::_textSize` and Terminal `lines`/`maxCharacters` are allocation metadata,
  not writable resize APIs. Grid rows/columns must be changed via `configure`,
  not direct assignment. Reconstruct Terminal to change buffer geometry.
- Text capacities remain byte-sized (0..255). Numkey preserves the existing
  `DISPLAY_SIZE` entry limit (currently six); `setNum` takes the **leading**
  `DISPLAY_SIZE` characters of its decimal representation, including a minus
  sign. It never promises a full-width INT_MIN/MAX in a six-character display.
- Grid IDs are 1..rows*columns; slot 0 is reserved and invalid lookups return 0.
  Zero dimensions or products above 255 produce an inert new grid. Invalid/OOM
  `configure` preserves the old grid; successful configure resets labels/states.
- Terminal clamps font size to 1..31 to keep byte-sized geometry representable,
  uses nonnegative dimensions, and caps line count at MAX_LINES. No writable
  interior means no lines. Partial line-allocation failure releases prior lines
  and leaves an inert terminal with zero lines.
- Drawing without an attached usable Canvas/TFT is a no-op. `show` restores
  visibility before drawing. Missing event handlers/targets and null touch
  pointers are safe. Canvas/widget lifetimes still follow the shared core's
  borrowing contract; these changes do not add automatic ownership of canvases.
- Normal C-string inputs must be NUL-terminated and readable for their stated
  use. Image pixel storage remains borrowed and must cover its declared extent.

## Final verification and limitations

- `python3 tests/host/run.py text`: PASS under ASan/UBSan/leak detection.
- `python3 tests/host/run.py`: PASS for charts, core, lifecycle and text in the
  final shared worktree, with no compiler warnings or sanitizer reports. An
  earlier full run briefly saw a format-truncation warning in the chart agent's
  concurrently changing `TrendBase.cpp`; the final run is clean.
- Shared allocation hooks inject C malloc/calloc/realloc failure, not C++ `new`
  failure. Button/Display use nothrow new and check null results; that particular
  allocation-failure branch is not injected by these tests.
- Host stubs establish memory safety and observable draw calls, not physical TFT
  timing, visual pixel equivalence, or AVR/ESP board-toolchain compatibility.
