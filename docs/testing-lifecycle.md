# Phase 1 lifecycle and registry safety

Run from the repository root:

```sh
python3 tests/host/run.py lifecycle
```

The host runner compiles the actual GUI `.cpp` sources, excludes hardware touch driver implementations, and enables AddressSanitizer, UndefinedBehaviorSanitizer, and leak detection. `lifecycle_drivers.h` supplies only hardware Touch subclasses so all four **real Canvas header implementations** can be exercised without SPI/I2C. It is not a hardware-driver test.

## Ownership and lifetime contract

- A Canvas exclusively owns its `Guitft* tft` and `Touch* ts`. Both start initialized; Touch has a virtual destructor. Canvas deletes both, including a derived Touch, exactly once. Public driver pointers remain for source compatibility: do not share them between canvases, replace them without releasing the old object, or assign stack objects.
- Canvas and Screen widget registries are **non-owning**. They never delete widgets. Screens are caller-owned too; Canvas never deletes `currentScreen`.
- Register a widget in one registry at a time. Remove it from its old Canvas/Screen before moving it. Duplicate adds within one registry are ignored. `add(nullptr, ...)` is ignored.
- Before destroying a widget while its registry remains usable, call `canvas.remove(&widget)` or `screen.remove(&widget)` (or pop it). Removal clears its drawing context and never draws or deletes it. Canvas pop preserves its historical erase/redraw behavior and now returns null for an empty registry; Screen pop is non-drawing and returns null when empty.
- Before destroying a Canvas while a Screen or widget will remain usable, remove Canvas widgets and call `screen.detach()` on **every associated Screen**, including inactive ones. `detach()` hides Screen widgets, disconnects their Canvas contexts, and clears the old Canvas's `currentScreen` if necessary. The Screen retains its non-owning widget registrations and may later receive another Canvas through `setCanvas`.
- Before destroying a current Screen while its Canvas remains usable, call `screen.detach()`. `canvas.setScreen(nullptr)` hides/deselects the current Screen but intentionally retains the Screen's Canvas association, matching screen-switching behavior; it is not a full lifetime detach.
- Destructors intentionally do **not** walk caller-owned widget/screen pointers. This avoids dereferencing already-destroyed stack objects during declaration-order teardown, but means dangling registrations cannot be discovered automatically. A widget's `canDraw()` guards a null context, not a previously freed context. No automatic Widget destructor detach is implemented.
- Canvas, Screen, and StackArray cannot be copied or copy-assigned. Move ownership is not introduced.
- Prefer `add/remove/pop` over direct writes to the public registries. Direct registry manipulation bypasses association management.

## Coverage and red/green evidence

Each safety slice was added and run before its fix:

1. Virtual Touch destruction initially failed its trait assertion; the real Canvas destructor now releases owned drivers and passes leak checks.
2. Safe default Canvas/Screen construction and null add/screen/pop/bounds operations initially failed dimension initialization.
3. A 300-widget dispatch test initially failed from byte-sized counts; Screen additionally used a signed-char index. Both now retain full registry indices.
4. Explicit remove/pop/detach APIs initially failed compilation because the APIs did not exist. Tests cover context clearing, duplicate registration, and noncopyability.
5. Injected failed realloc originally entered the legacy permanent LED loop (caught by the test's watchdog). Tests now verify pointer/data/count preservation, successful retry, safe empty access, initial allocation failure recovery, and failed add with no partial widget mutation.
6. Repeated initialization initially replaced/leaked drivers. All four Canvas implementations now preserve the original Touch pointer and initialize hardware only once after successful startup. Constructor-triggered startup remains intact.
7. Scan timestamp/rollover assertions initially failed. The timestamp now has `unsigned long` width and elapsed-time subtraction handles wraparound. Touch sampling uses the same subtraction form, preserving the existing strict `>` threshold.
8. Poisoned-storage Slider construction initially exposed an uninitialized callback/timestamp. Tests cover null callback/point and unattached drawing safety.
9. Injecting a null registry entry initially produced a UBSan null dereference. Traversals now skip null entries and null touch points.

`lifecycle_odr.cpp` is a second translation unit including all four Canvas implementations plus Slider. Linking it with `test_lifecycle.cpp` initially failed with multiple Slider definitions; all out-of-class header implementations are now inline. Parent-owned Arduino multi-TU tests separately cover the real hardware-facing XPT header.

## Deliberately deferred

- No flash/device/SPI/I2C execution or calibration validation was performed.
- STMPE610's existing blocking begin/retry loop and constructor-triggered startup are retained for compatibility; a disconnected controller may still block startup.
- Input repeat/debounce/event propagation semantics, callback mutation of registries, and reentrant destruction during dispatch remain later-phase work. Do not mutate/destroy the dispatched registry/widgets inside callbacks.
- StackArray continues its original malloc/realloc storage model for scalar/pointer-like values; it is not a general object-lifetime-aware STL container.
- Allocation injection covers C allocation/reallocation in registries, not C++ driver `new`; driver allocation uses `std::nothrow` and null guards, but an allocator-failed hardware startup was not emulated.
- An `add` retains its legacy void interface; failure is a safe no-op, observable via registry count, not a new error-return API.
