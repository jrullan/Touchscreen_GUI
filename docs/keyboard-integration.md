# Keyboard selective-port integration

Base: default branch after merged PR8 (`be8047a`); working branch `feat/keyboard-safe-port`.

## Intended scope

Port v2's QWERTY lower/upper/symbol layouts while separating command actions from printable characters. Provide fixed-capacity ASCII editing, read-only text views, safe append/backspace/clear, explicit accept/cancel result and callback, and bounded target Button updates. Preserve the existing direct-drawing library and Numkey APIs; do not add region redraw or global event-dispatch redesign merely to support this widget.

The keyboard must remain safe when unattached/hidden, with tiny or invalid public geometry, allocation failures in inherited Button state, target capacities smaller than edited text, repeated show/hide and registry removal. No silent target truncation on accept. Borrowed target lifetime must be explicit. Cancel must not modify target text. Literal `#`, `^`, `<` and backslash must not accidentally trigger commands.

Command rows and layout changes from v2 must be documented. Keep callbacks outside any TFT transaction and perform no widget member access after a callback that may remove/destroy the widget. Respect the existing consume/propagate return convention and unsigned debounce rollover. Do not overwrite or remove unrelated widgets on dismissal.

## Validation gates

- Test-first production editing/actions/geometry/target/ownership regressions and full host sanitizers.
- Real Adafruit GFX raster coverage: labels and text must remain within valid drawing bounds, including normalized font zero and screen-edge wrapping.
- Clean exact-library ESP8266 builds for the keyboard and old demos/dashboard, preserving rollback.
- Independent source review before device readiness.
- Brief device demo checks typing, literal special characters, modes, deletion, capacity boundary, accept/cancel and reopening. Restore panelboard afterward.

Implementation and validation are in progress; this document is not evidence of completed gates. The panelboard remains untouched during development.
