# Keyboard device acceptance checklist

Status: demo flashed; user reported refresh artifacts while typing. Ordinary-touch repaint was narrowed to the text-field interior, rebuilt and flashed; user then confirmed `verified`. This records physical acceptance of the typing-refresh correction, not an itemized claim that every check below was completed. Panelboard restored afterward: upload hash verified, boot and WiFi confirmed, four HTTP 200 polls, no crash/watchdog markers in the observation window.

## Test vehicle

`tests/arduino/keyboard/keyboard.ino` uses the opt-in polling XPT2046/mapper Canvas, TFT CS16/DC15 and touch CS0 on the D1 Mini. It has no network or credentials. Target button is at y8; keyboard occupies y128..319. Tapping the target reopens the editing session. Completion prints result, editor text, target text and free heap over 115200-baud serial. Heap snapshots are not leak-proof endurance evidence.

## Checks requiring physical observation

1. Startup renders target and keyboard with no crash/watchdog.
2. Type lowercase, toggle shift and type uppercase; switch to symbol mode.
3. Type literal `#`, `^`, `<`, and backslash by their symbol cells: they must be characters, not commands. Dedicated shift and symbol command keys are at the outer ends of the last character row.
4. Backspace removes one character; space inserts a literal space.
5. OK updates the target before the completion notification. Reopen by tapping target and confirm committed text is prefilled.
6. Change the text, Cancel, reopen: target remains unchanged.
7. Fill to 48 characters, attempt one more, then delete and type again: stay bounded and responsive.
8. Repeat accept/cancel/reopen. Confirm no duplicate key dispatch or stale keyboard pixels after dismissal. Held keys intentionally repeat after the debounce interval.
9. Restore the saved panelboard firmware; verify boot, WiFi association and successful HTTP polls. Physical visual acceptance must come from the user, not serial alone.

## Build evidence after font correction

Compatibility matrix passed 11 clean exact-library ESP8266 builds (including keyboard and panelboard) after the classic-glyph forwarding correction. The existing repository demo also clean-built with its explicit Neotimer dependency supplied. Legacy demo emits existing mutable-string warnings. All six keyboard real-GFX raster cases passed in the integration-owner run; independent final review is pending. No device upload or physical acceptance is implied.

Artifacts: profile scratch `gui-keyboard-builds/`, keyboard and dashboard output directories. Never flash while serial monitor holds the port. Do not treat an upload hash check as visual or touch acceptance.
