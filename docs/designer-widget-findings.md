# Designer widget findings: integration scope

Working branch: `fix/designer-widget-findings`, base default `cd9294b`.

## Approved corrections

- Portable icon declarations for architectures beyond AVR/ESP8266/ESP32.
- ESP8266 icon flash storage, preserving pgm_read_word reads.
- Explicit IconButton default debounce of 500 ms, retaining setDebounce(0). This limits repetition, not one-toggle-per-contact edge semantics.
- Geometry-aware X label selection in shared TrendBase for Trend and MultiTrend.
- First HIGHLIGHT press and first press after clear in non-maintained Buttongrid.

## Preserve legacy contracts

- IconButton callback explicitly calls update if it wants immediate image refresh. No unconditional redraw after callbacks that may switch screens.
- Numkey completion reports entry to its callback; no automatic write into the target button. Explicit targetEventHandler bridge remains explicit.
- Dial ring uses fgColor; ticks and endpoint labels use borderColor. Do not swap these silently.

## Gates

Each correction requires a failing production-source regression before the minimal fix. Run full host sanitizers, strict checks, actual-GFX raster tests, transactions and bundled XPT tests. Add icon portability test to CI and Arduino compatibility fixture. Clean-build exact library and repository demo, compare final icon memory placement, obtain independent review. Host preprocessor syntax success does not validate full hardware support on STM32/RP2040/SAMD.

## Completed verification

- Full host suite passed under ASan/UBSan/leak detection: 23 executables.
- Icon tool tests passed for six architecture macros; actual Xtensa section tests verified flash placement for all six assets. Icon payloads unchanged.
- Strict checks, existing Display/bar/Terminal/Keyboard raster runners, transaction and bundled XPT regressions passed.
- Independent review first reproduced inherited-custom-font X-axis overflow. Correction renders explicit classic glyphs through a stack-local forwarder without altering caller text state.
- Integration-owner and independent reruns passed 361 real-GFX axis cases per chart, plus 11,880 host geometry cases per chart.
- Final independent rereview passed with no security concerns or logic blockers. Four-rotation axis raster coverage remains a non-blocking suggestion.
- After font correction, eleven exact-library clean ESP8266 matrix builds passed, including panelboard, plus the repository demo with explicit Neotimer dependency. The demo retains legacy mutable-string warnings.
- Demo static RAM reported 31,504 bytes versus the earlier identical demo's 48,704 bytes before icon flash placement. This is compiler static-RAM accounting, not measured runtime heap/endurance.
- Icon portability and axis real-GFX regressions added to CI; hosted CI has not run for these unpublished changes.

No Designer edits, firmware upload, commit or push performed. Existing designer and running panelboard remain untouched. Real STM32/RP2040/SAMD board builds and physical acceptance are not claimed; STM32 axis compatibility was checked only at host syntax/API level.
