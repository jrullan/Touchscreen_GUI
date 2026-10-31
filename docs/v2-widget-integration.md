# Selective v2 widget integration

Baseline: c883292 (our safety and chart/Display rendering branch). This is not a wholesale v2 merge.

## Included

- Gauge/Slider retain last committed geometry/style/value and draw delta strips.
- Terminal retains exact row text/colors and repaints only changed rows; safe full-composition fallback handles overflow/overlapping glyphs and snapshot allocation failure.
- Guitft supports bounded nested pixel transactions; widgets use scopes only when drawing is necessary.
- CI covers host sanitizers, pinned real-GFX Display and widget raster tests, and isolated transaction dispatch tests.

Two independent source reviews found no confirmed blockers. Follow-ups clarified base nonvirtual hide/Terminal allocation contracts, backend-internal yields and display-control command exclusions. Real STM32 compilation and long-duration WiFi/shared-bus stress are not established by this change.

## Parent-executed validation

All host suites, strict return checks, exact-build-command regression, Display rasterizer, new real-GFX widget raster/transaction runner, standalone transaction suite, whitespace checks, seven clean ESP8266 matrix builds and repository demo compilation passed. Demo compilation retains legacy string-literal-to-char-pointer warnings. The demo has not been physically accepted for this integration yet.

## Device measurement

Identical `examples/Widget_Delta_Benchmark` source was compiled clean against baseline and candidate and run to completion on ESP8266. No WiFi. Board timings include widget work, not serial logging or inter-call delays. This is a single deterministic run per revision, not a statistical multi-run speed guarantee. See `v2-widget-device-benchmark.json` for every sample and source hash.

| Workload | Baseline median us | Candidate median us |
|---|---:|---:|
| Gauge changed (80) | 2397.5 | 127 |
| Slider changed (80) | 2604 | 101 |
| Gauge unchanged (40) | 2393 | 12 |
| Slider unchanged (40) | 2604 | 8 |
| Terminal print (20) | 16621 | 15814.5 |
| Terminal unchanged (20) | 12109 | 76.5 |

The changed-bar workload mostly moves by two percentage points, with a few threshold crossings/large changes. Do not generalize its median to full rebuilds. Terminal print includes both initial appends and full software scrolling; it is not uniformly faster by a large margin. Backend nesting is only part of the candidate: these are combined widget+nesting results, not SPI-only gains.

Target object sizes baseline/candidate: Gauge 96/204, Slider 108/168, Terminal 124/288 bytes. Free heap after workload 50472/49984; largest block 50456/49968. These are post-workload values, not peak or long-term fragmentation evidence. Terminal additionally owns a row snapshot; static build totals exclude such dynamic allocations.

After the benchmark the SPI visual comparison was restored with a 10 ms frame delay. Physical observation of that comparison found no obvious visual difference; serial drawing times were around 18.5 ms ungrouped and 18.1 ms nested. It uses already-optimized Adafruit fills. Do not claim v2's historical bulk-write multiplier applies here.

## Deferred

No v2 primitive overrides/writeColorFast, TouchMapper, Keyboard, circular histories or hardware scrolling are included. No external push/PR or installed-library replacement is implied by this local validation. Display-control calls such as rotation/inversion/scroll commands must run outside ScopedWrite. It is not a shared-bus lock.
