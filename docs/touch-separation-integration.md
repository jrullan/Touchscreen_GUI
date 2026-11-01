# Touch separation: integration scope

Base: merged Touchscreen_GUI PR6 (`1dbc70c`). Working branch `refactor/touch-driver-mapper`.

## First slice

Introduce an opt-in raw hardware interface (`TouchDriver`), calibration/mapping/filtering/state component (`TouchMapper`) and `Canvas_Touch` adapter with an XPT2046 driver. Keep existing `Canvas_XPT2046`, `Canvas_FT6206`, `Canvas_STMPE610` constructors and defaults unchanged. Do not replace the operating dashboard or installed library during implementation.

The mapper and Canvas borrow their dependencies. Lifetime order must be hardware driver, mapper, Canvas, with reverse destruction. Every failure path must remain inert until initialization succeeds, and retries must be leak-free/idempotent. Do not publish failed hardware through legacy `ts`.

Rotation must remain synchronized between TFT geometry and touch mapping after initialization and runtime orientation changes. The opt-in mapper uses canonical portrait pixel extents 0..239 and0..319; existing legacy mapping uses0..w and0..h and rejects coordinates beyond screen edges. Keeping the legacy path untouched avoids silently changing its established calibration behavior; document the opt-in difference and physically validate corners before migrating an app.

No gesture/event propagation redesign, storage rewrite, mandatory global allocation, alternate hardware ports or hardware calibration persistence is included.

## Verification status

Parent-executed full host sanitizers (15 suites), strict checks, real-GFX raster/widget/transaction runners, the existing ESP8266 compatibility matrix, the opt-in touch fixture and repository demo compilation passed. A scratch copy of the repository demo was adapted only at the Canvas include/declarations to use the opt-in mapper. It was clean-built and flashed with upload-hash verification. The user then confirmed “Everything worked” after being asked to check buttons, keypad, Slider alignment and screen switching. This accepts the portrait demo path, not every physical rotation. The panelboard was subsequently clean-built against this branch, restored and observed booting, joining WiFi and completing four HTTP200 polls without exception/watchdog markers. Dashboard serial evidence is in active profile scratch `gui-touch-device/dashboard-serial.log`.

Independent review initially found an IRQ teardown defect. The additive adapter now always constructs polling hardware and rejects non-255 IRQ startup before initialization. Independent re-review approved the fix without remaining confirmed blockers. `tests/xpt2046/run.py`, now included in CI, compiles the actual bundled XPT source and verifies rejected IRQ/polling lifetimes, 16 rotated corners and closest-pair filtering with the 3 ms cache. Its sanitizer tests and all 15 host suites passed after the fix. Physical calibration remains a separate acceptance gate.

The actual-source reviewer probe also checked all 16 rotated calibration corners without finding double rotation. Physical calibration remains unverified.

## Acceptance gates (pending implementation)

- Real production mapper tests: null/failed startup, retry/idempotence, invalid calibration, descending raw axes, swap, all rotations, edge/outlier coordinates, filtering, state transitions and timing wrap.
- Canvas tests: legacy routing unchanged; new path cannot dispatch pre-init or release samples; no double ownership; runtime geometry/rotation matches touch bounds.
- Full existing host sanitizers/strict checks, actual-GFX raster/transaction coverage and clean exact-library ESP8266 builds, including old demo/dashboard plus new opt-in fixture.
- Independent review before readiness claim.
- Parent-controlled physical touch validation, then restore the dashboard. Do not describe target compilation as physical touch acceptance.

User reports the previous panelboard version ran over one day. This is real-world normal-operation endurance evidence, not measured heap stability or deliberate outage recovery. Controlled recovery tests remain separate.
