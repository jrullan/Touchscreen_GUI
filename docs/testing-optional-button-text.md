# Optional inherited Button text storage

## Contract and usage

All old constructor call forms keep their old behavior when rebuilt from source.
This change preserves source compatibility, not precompiled constructor symbols;
rebuild the library and application together (use clean Arduino builds). `Button()`, both old
Keyboard forms and IconButton allocate an owned `Text(8)` and set its value to
`"Button"` when allocation succeeds. Existing Button sized/string/round
constructors are unchanged. This is an explicit opt-in, not a new default.

```cpp
using TextStorage = Button::TextStorage;
Keyboard keyboard(TextStorage::None);
Keyboard styled(240, 192, BLACK, WHITE, RED, TextStorage::None);
IconButton icon(50, 50, imageOff, imageOn, (char*)"Lamp", TextStorage::None);
// No label allocation either:
IconButton bare(50, 50, imageOff, imageOn, nullptr, TextStorage::None);
```

`TextStorage::Default` explicitly selects the legacy allocation policy.
The policy is consumed during construction, not stored in the object. There is
no extra member, framebuffer, drawing-state cache or lazy allocation.
`Button::init()` still performs all its normal shared initialization; its
`setText("Button")` cannot allocate when contents is null.

With `None`, **public inherited `contents` is null by contract**. Do not directly
dereference it, delete it, or rebind it. Inherited `Button::getText()` returns
null, `getNum()` returns zero, and both `setText` overloads, `drawText()` and
`fitToText()` safely do nothing, as with the existing null/OOM behavior. Normal
geometry/style initialization, image rendering, labels and event routing remain.
Label ownership/allocation is independent; IconButton's legacy omitted label is
an empty owned string (one requested byte), not a null label.

`Keyboard::getText()` hides the nonvirtual `Button::getText()` and returns its
independent fixed edit buffer. `Keyboard::setText()` edits that buffer; through
`Button*`, `setText()` remains the inherited no-op with `None`. A Keyboard using
`None` can edit an ordinary Button target and accept/cancel normally. A target
whose inherited contents is null (including another opt-out widget) is rejected
by the existing Keyboard session validation; this does not create target storage.

## Vertical RED / GREEN evidence

Executed against real production sources on `perf/optional-button-text`, starting
at `410823d`, before changing the behavior under test:

1. Added the IconButton opt-out contract test first. Running
   `python3 tests/host/run.py optional_button_text` failed to compile because
   `Button::TextStorage` did not exist.
2. Added only the named policy/constructor wiring, initially preserving the
   legacy allocation. The same command then built and **failed at runtime**:
   `test_optional_button_text.cpp:10: Assertion 'lean.contents==nullptr' failed`
   (SIGABRT). This proves the real inherited allocation was present.
3. Changed the policy constructor to skip `new Text(8)` for `None`, while still
   calling shared `init()`. Same test: PASS under ASan/UBSan/leak detection.
4. Added the Keyboard tracer (edit buffer, Button-target accept/cancel and styled
   opt-out). Same command failed with `no matching function for call to
   Keyboard::Keyboard(Button::TextStorage)` and the six-argument styled form.
   Implemented forwarding constructors; same command: PASS.
5. Added allocation/retention test before extending instrumentation: build failed
   because `host_heap_begin/stats/end` did not exist. Implemented the measurement
   seam; the test passed. Expanded regression coverage and refactored only after
   these vertical slices were green.

Later regression tests additionally cover old constructors, mutable/const base
setters, default Text-object new failure, default buffer calloc failure, opt-out
with both allocation failures armed (neither consumed), label OOM, polymorphic
destruction, actual touch editing/mode/backspace/space/accept/cancel, rejected
null-storage targets, and no lazy allocation during repeated initialization.

## Host measurement (not MCU heap usage)

`python3 tests/host/run.py optional_allocations` measures successful allocations
and requested bytes while each stack widget is alive, with the same constructor
arguments on each side except the explicit policy. Existing C allocation hooks
are extended only for this executable with linker wrappers for scalar C++ new,
unsized/sized delete and free. `malloc`, `calloc` and `realloc` are counted too;
calibration checks each C route and ordinary scalar new/delete. The Text path is
nothrow new -> scalar new, plus calloc. Real allocator functions are used (including
ASan new/delete pairing); there are no fabricated sizes or malloc-only estimates.
A fixed nonallocating pointer table tracks retention and aborts on overflow.

| Constructor case | Default allocations / requested / retained | None allocations / requested / retained |
|---|---:|---:|
| Keyboard, default geometry | 2 / 25 / 25 | 0 / 0 / 0 |
| Keyboard, styled geometry | 2 / 25 / 25 | 0 / 0 / 0 |
| IconButton, null label | 2 / 25 / 25 | 0 / 0 / 0 |
| IconButton, empty label | 3 / 26 / 26 | 1 / 1 / 1 |
| IconButton, `Lamp` label | 3 / 30 / 30 | 1 / 5 / 5 |

Every case returns to zero tracked retained bytes after destruction. Plain
Button() is also measured at 2 allocations / 25 requested / 25 retained.
The host Text allocation is 16 requested bytes, its Text(8) buffer 9. The omitted
pair therefore removes 2 allocations / 25 requested bytes **on this host**.
This excludes allocator headers, fragmentation, stack storage and MCU effects.

Observed host object sizes: `Button=104`, `IconButton=128`, `Keyboard=192`,
`Text=16` bytes. The policy adds no member. Do not convert these host sizes or the
25 requested-byte delta into ESP8266 heap-saving claims; target allocation/probe
verification is a separate integration task.

## Reproduction and verification

```sh
python3 tests/host/run.py
python3 tests/rasterized/run_keyboard.py
python3 tests/rasterized/run_icon_runs.py
python3 tests/host/run_optional_raster.py
python3 tests/host/check_returns.py
python3 tests/tools/test_icons.py
python3 tests/tools/test_check_arduino.py
python3 tests/transactions/run.py
python3 tests/xpt2046/run.py
g++ -std=c++17 -Werror=return-type -Werror=write-strings -Werror=conversion-null \
  -DARDUINO=10819 -DESP8266 -Itests/host/stubs -I. -fsyntax-only \
  Button.cpp IconButton.cpp Keyboard.cpp
git -c core.whitespace=cr-at-eol diff --check
```

All commands above passed. Host suites use ASan/UBSan and leak detection.
Both default and opt-out real-GFX runs passed all six Keyboard cases (`labels`,
`restore`, `edges`, `modes`, `customfont`, `font0`) and 11,358 icon oracle cases.
`run_optional_raster.py` redirects the existing real-GFX runners' test compilation
to scratch copies with only constructors changed to None and null-contents
assertions added. It does not replace the pixel oracles or modify raster fixtures.
The production library sources and real installed Adafruit_GFX.cpp are compiled.
GFX source SHA256:
`5047f01e8e6dd3029c73cb9447a915fc06675d2e1cc9ebf6a6c1126f556fcecd`.

Button files retain their existing CRLF endings; the whitespace check therefore
uses Git's `cr-at-eol` option. A C++11 syntax probe also completed but emitted
existing C++17-inline-variable warnings from host stubs; the clean strict check
uses C++17 like the host runners. Integration additionally passed the clean
exact-library ESP8266 build matrix, including the new `optional_text` fixture,
the existing dashboard and the original repository demo. The opt-out fixture
compiles both Keyboard forms, IconButton, inherited null-safe APIs and normal
Button-target editing against the real target toolchain. CI runs the opt-out
raster suite as well as the host allocation tests. No firmware was uploaded;
physical opt-out acceptance and MCU heap measurements remain unverified.
No Designer files were changed.
