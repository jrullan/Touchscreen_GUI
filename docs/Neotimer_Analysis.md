# Neotimer — Role Analysis in Touchscreen_GUI

## Date: February 2026

## What is Neotimer

Neotimer is a non-blocking timer library for Arduino microcontrollers. It wraps `millis()` comparisons into a clean object API for scheduling actions without calling `delay()`. Created as a companion utility for embedded projects where blocking is unacceptable.

Source: single header-only implementation (`neotimer.h`), no dependencies beyond `Arduino.h`.

## Usage in the Framework

Neotimer is used **exclusively in example sketches** — it is not a dependency of the library itself.

### Examples that use it

| Example | Timer | Interval | Purpose |
|---------|-------|----------|---------|
| Touchscreen_GUI_Demo | `dialTimer` | 5000 ms | One-shot delay before dial animation starts |
| Touchscreen_GUI_Demo | `incrementTimer` | 10 ms | Periodic: step the slider animation every 10ms |
| Touchscreen_GUI_Benchmark | `dialTimer` | 5000 ms (set to 0) | Same as demo, overridden to 0 for immediate start |
| Touchscreen_GUI_Benchmark | `incrementTimer` | 10 ms | Periodic: step the animation every 10ms |
| maker_faire_pip | `trendUpdateTimer` | 10 ms | Periodic: feed new values to the Trend widget |
| maker_faire_pip | `terminalUpdateTimer` | 250 ms | Periodic: update terminal display |
| Demo_MEGA_SEEEDTOUCH | `dialTimer` | 5000 ms | Same one-shot delay pattern |
| Demo_MEGA_SEEEDTOUCH | `incrementTimer` | 10 ms | Same periodic animation pattern |

### Two usage patterns

**1. One-shot delay** — `start()` then poll `done()` in the loop:
```cpp
Neotimer dialTimer = Neotimer(5000);
// ...
dialTimer.start();
// In loop:
if(dialTimer.done()) { /* proceed */ }
```

**2. Periodic scheduling** — `repeat()` returns true every N ms, auto-resets:
```cpp
Neotimer incrementTimer = Neotimer(10);
// In loop:
if(incrementTimer.repeat()) { /* runs every 10ms */ }
```

## How the Framework Handles Timing Internally

The library's widgets implement their own `millis()`-based timing inline — functionally identical to what Neotimer does, but hand-rolled:

| Component | Pattern | Code |
|-----------|---------|------|
| Button debounce | `if(lastMillis + debounceTime < millis())` | Button.cpp:259 |
| Slider debounce | `if(lastMillis + debounceTime < millis())` | Slider.h:95 |
| Buttongrid debounce | `if(lastMillis + debounceTime < millis())` | Buttongrid.cpp:191 |
| Numkey debounce | `if(lastMillis + debounceTime < millis())` | Numkey.cpp:258 |
| Popup debounce | `if(lastMillis + debounceTime < millis())` | Popup.cpp:135 |
| Canvas scan sampling | `if(millis() > (scanSampling + scanSampleDelay))` | Canvas.cpp:99 |
| Canvas_XPT2046 touch sampling | `if(millis() > touchSampling + TOUCH_SAMPLING_TIME)` | Canvas_XPT2046.h:63 |
| TouchMapper press duration | `millis() - _pressStartTime` | TouchMapper.cpp:54 |

These are all single-comparison debounce/throttle checks — the simplest case of what Neotimer provides.

## Assessment

### Adequate for its role: Yes

Neotimer fits well as an example-level utility. It solves the right problem (non-blocking scheduling) with minimal overhead (one `millis()` comparison per `done()`/`repeat()` call). The API is readable and the two patterns used in the examples (`done()` for one-shot, `repeat()` for periodic) are clean.

### Strengths

- **Zero overhead when not firing** — just a `millis()` subtraction and comparison
- **`repeat()` auto-resets** — exactly what the animation loops need, no manual `reset()`/`start()` bookkeeping
- **No dynamic allocation, no interrupts** — safe for any Arduino platform
- **Readable in examples** — `if(timer.repeat())` is immediately understood

### Minor observations

| Item | Detail |
|------|--------|
| **`long` vs `unsigned long`** | `_timer.time` and `_timer.last` are `long` (signed 32-bit). `millis()` returns `unsigned long`. For intervals under ~24 days this is fine, but `unsigned long` would be technically correct. |
| **Header-only implementation** | All method bodies are in `neotimer.h`. This works in Arduino's single-translation-unit build system but could cause multiple-definition linker errors in standard C++ multi-file projects. |
| **Not used in the library itself** | The framework's internal timing (widget debounce, scan sampling) uses inline `millis()` checks rather than Neotimer. This is appropriate — those are single-condition checks where an object wrapper adds no clarity. |
| **`repeat()` first-call behavior** | On the first call, `repeat()` starts the timer and returns false. The first true return comes after the interval elapses. This is correct but worth noting — there's always a one-interval startup delay. |

### Conclusion

Neotimer is a good fit for its current role: scheduling non-blocking periodic and one-shot actions in user sketches. There is no compelling reason to replace it with a different timer library, nor to pull it into the framework internals — the library's inline `millis()` debounce checks are simpler and more appropriate for their single-condition use cases.
