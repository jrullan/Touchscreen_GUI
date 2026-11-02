# Widget memory and redraw optimizations

This change combines four optimizations without replacing the direct-drawing architecture.

## Defaults and opt-in paths

Keyboard mode repaint and compact chart caches are automatic. Existing constructor calls remain source-compatible. Clean-rebuild the library and applications: constructor symbols and some class layouts changed; precompiled ABI compatibility is not promised.

Existing chart histories stay contiguous newest-first by default. Terminal row addresses stay stable by default. Storage-changing paths require explicit opt-in:

```cpp
Trend trend(180, 140, 0, 50, 100, 32, HistoryStorage::Circular);
MultiTrend multi(180, 140, 0, 50, 100, 3, 32, 3,
                 HistoryStorage::Circular);
Terminal terminal(230, 80, TERMINAL_SCROLL_DOWN, 1,
                  Terminal::ScrollStorage::RotateRows);
```

Circular charts expose a physical ring through `values`; use `trend.sample(index)` or `multi.sample(series,index)` for logical newest-first reads. Each MultiTrend series advances independently. No mirror buffer or pre-render history linearization is used.

Rotating Terminal callers must reacquire `linesBuffer[row]` after scrolling or printing. All logical row bytes and colors, including duplicated-edge scroll behavior, remain intact. This is lower-copy scrolling, not zero-copy scrolling.

## Observed host results

- Keyboard alphabet Shift: 90 full-frame fills versus 27 selective fills; real-GFX pixel writes 82,135 versus 15,520 in the recorded fixture. Symbol-mode Shift can repaint only its indicator. No new object fields or allocations.
- Chart cache, three active series and 32 samples: 548 versus 274 requested bytes. Coordinates and colors are validated before compact storage; out-of-range frames use lossless native-int storage. A previously enlarged allocation is retained until init rather than shrunk on every format change.
- Circular chart insertion writes one history sample instead of shifting the row. Host MultiTrend object size increases from 408 to 416 bytes for independent heads/policy; Trend and TrendBase host sizes are unchanged. Heap histories are not duplicated. Sample-write counts are not timing benchmarks.
- Terminal, 200 mixed-direction scrolls with five 18-byte rows: 800 versus 200 copies, 14,400 versus 3,600 copied bytes. Both policies retain 532 requested bytes and perform zero workload allocations in this fixture.

These are measured host requested bytes and instrumented drawing/copy operations, not board free RAM, physical SPI transactions or measured hardware speedups.

## Verification

All 30 host sanitizer executables passed. New real-GFX tests passed 704 Keyboard transitions, 652 Terminal full-panel comparisons, and chart compact/wide/circular/default equivalence tests. Existing widget, capacity (816 full-panel comparisons), X-axis, icon, transaction and bundled-XPT regressions passed. CI includes all new raster suites and Terminal copy/heap assertions.

Clean exact-library ESP8266 builds passed the compatibility matrix, the combined opt-in fixture, dashboard and original repository demo. Two independent review scopes approved the changes. No firmware was uploaded; physical acceptance and board timing remain unverified.

Independent chart review also identified incremental-versus-fresh differences for some extreme-offscreen/marker sequences in the unchanged baseline. These are not newly introduced regressions and are not fixed or claimed solved by this optimization. Boundary behavior beyond the executed cases and AVR runtime behavior are not universally validated.

Detailed contracts and reproducible commands:

- [Keyboard mode repaint](testing-keyboard-modes.md)
- [Chart cache and history](testing-chart-storage.md)
- [Terminal scrolling](testing-terminal-scroll.md)
