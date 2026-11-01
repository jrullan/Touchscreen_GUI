# Physical XPT2046 rotation acceptance

After merging PR7, a parent-controlled guided fixture used the opt-in Canvas_Touch, TouchMapper and polling TouchDriver_XPT2046 on the ESP8266/ILI9341/XPT2046 device. It requested four inset corner targets and center for physical rotations0,1,2,3, advancing only after a nearby press and release, then allowed free drawing.

User reported: **"Completed all tests and tried out drawing. All worked"**. This accepts the guided target sequence and drawing in all rotations by direct user observation. Fixture source/build artifacts are in the active profile scratch `gui-touch-rotations/`.

The later serial query toggled/reset the board: the returned log showed a fresh boot and mask0 rather than retained completion. Therefore no serial mask is claimed as independent completion evidence. Capture logs continuously before user interaction in future, or persist results explicitly if reset-safe retrieval is required.

The panelboard was restored from the retained clean build and its upload hash verified. Serial showed boot, WiFi192.168.1.162 and four HTTP200 polls without exception/watchdog markers. This is restoration evidence, not a new long-duration endurance run.

Validation is recorded and read back on https://github.com/jrullan/Touchscreen_GUI/pull/7#issuecomment-6560991397.

FT6206 and STMPE610 hardware are not connected; their upcoming additive adapters require explicit host/compile-only validation labels. Existing legacy controller canvases stay unchanged.
