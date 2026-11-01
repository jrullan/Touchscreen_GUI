# Additional touch adapter integration

Base: merged PR7 (`0fddcf1`), branch `feat/touch-driver-adapters`.

## Scope

Add opt-in FT6206 (I2C) and STMPE610 (hardware SPI) adapters to the existing TouchDriver/TouchMapper/Canvas_Touch architecture. Preserve legacy Canvas classes and current panelboard behavior. Controllers are owned by value; mapper and Canvas keep the existing borrowed lifetime contract.

Both adapters attempt startup once per call, retry failure without allocation, and become idempotent after success. Before readiness, touched/count/raw reads remain inert with zero output. FT uses the explicit indexed point getter to avoid the bundled overload ambiguity. STM validates signed-pin storage and reads one oldest FIFO record with readData, rather than calling the bundled getPoint that drains FIFO and has undefined empty output. Neither adapter adds IRQ support or mapper median filtering across event records.

## Calibration

New nominal presets preserve legacy portrait axis directions using inclusive valid pixels:
- FT6206: `{239,0,319,0,false}`. Nominal 240x320 panel, not measured calibration.
- STMPE610: `{3775,240,240,3700,false}`. Uses existing experimental raw endpoints with portrait X flipped.

The new mapper composes consistent rotations from canonical portrait; this is not a promise of byte-for-byte legacy landscape behavior. Old hardware canvases are not migrated. Runtime calibration remains available.

## Parent verification

All host sanitizer suites including new adapters and preset corner checks passed. Strict checks, clean-build command regression, actual bundled XPT source tests, real-GFX Display/widget raster tests and SPI nesting suite passed. Preset tests checked 32 rotated nominal corners; these are mathematical host tests, not physical panel evidence.

Ten clean ESP8266 matrix builds passed: display, input, terminal, trend, sizes, XPT mapper, FT6206 adapter, STMPE610 adapter, instrumentation, dashboard. New fixtures compile the actual bundled controller implementations and APIs, whereas adapter host unit tests substitute their hardware boundary. Builds did not flash anything.

FT6206 and STMPE610 hardware are not connected. Their electrical behavior, calibration, contact/read races, STM FIFO backlog/overflow handling and physical rotation accuracy remain unverified. Independent review approved without confirmed blockers; documentation follow-ups clarify the integrated defaults and retained sampling limitations. Hosted CI has not yet run on this branch. The XPT device's prior four-rotation/drawing acceptance is recorded separately in testing-touch-rotations-device.md and does not validate these other controllers.
