// ============================================================
// Sample_Button — Button Widget Reference
// ============================================================
// Demonstrates the Button widget with various configurations:
//   - Default button with event handler
//   - Styled button with custom colors and corner radius
//   - Toggle button that changes appearance on press
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// Wiring:
//   ESP8266: TFT_CS=D0(16), TFT_DC=D8(15), TS_CS=D3(0)
//   ESP32:   TFT_CS=26,     TFT_DC=5,      TS_CS=17
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Button.h>
#include <Terminal.h>

// ---- Pin Configuration ----
#ifdef ESP32
  #define TFT_CS 26
  #define TFT_DS 5
  #define TS_CS  17
#else
  #define TFT_CS 16
  #define TFT_DS 15
  #define TS_CS  0
#endif

// ---- Touch & Canvas ----
TouchDriver_XPT2046 touchDriver(TS_CS);
TouchMapper mapper(&touchDriver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, TFT_CS, TFT_DS, &mapper);

// ---- Widgets ----

// Button() — default constructor allocates 8-char text buffer.
// Text buffer auto-grows via realloc if setText() receives longer text.
Button btnBasic = Button();

// Button() — another default button, will be styled differently
Button btnStyled = Button();

// Button() — toggle button that changes state on each press
Button btnToggle = Button();

// Terminal to show button events
Terminal terminal = Terminal(232, 80, TERMINAL_SCROLL_UP, 1);

// ---- State ----
uint16_t pressCount = 0;

// ---- Event Handlers ----
// Button event handlers receive a pointer to the Button that was pressed.
// Use btn->touched to read the toggle state (flips on each press).

void onBasic(Button* btn) {
  pressCount++;
  char buf[24];
  sprintf(buf, "Basic pressed: %d", pressCount);
  terminal.print(buf, GREEN);
}

void onStyled(Button* btn) {
  terminal.print("Styled pressed!", CYAN);
}

void onToggle(Button* btn) {
  // btn->touched is automatically toggled by the framework on each press
  if (btn->touched) {
    btn->setColors(GREEN, BLACK, WHITE);   // Active state
    btn->setText("ON");
    terminal.print("Toggle: ON", GREEN);
  } else {
    btn->setColors(GRAY2, WHITE, WHITE);   // Inactive state
    btn->setText("OFF");
    terminal.print("Toggle: OFF", RED);
  }
  btn->show();  // Redraw button with new colors/text
}

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  canvas.init();
  Guitft* tft = canvas.tft;

  // Title
  tft->setTextColor(WHITE);
  tft->setTextSize(1);
  tft->setCursor(4, 4);
  tft->print("Sample: Button Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // BUTTON CONFIGURATION REFERENCE
  // ============================================================

  // --- Basic Button ---
  // setColors(bgColor, fgColor, borderColor)
  //   bgColor:     button fill color
  //   fgColor:     text color
  //   borderColor: border line color
  btnBasic.setColors(BLUE, WHITE, WHITE);

  // setSize(width, height) — button dimensions in pixels
  btnBasic.setSize(110, 36);

  // fontSize — text size multiplier (1=6x8, 2=12x16, etc.)
  btnBasic.fontSize = 1;

  // setText(text) — sets the button label text
  // Buffer auto-grows if text exceeds initial allocation
  btnBasic.setText("BASIC");

  // borderWidth — border thickness in pixels (default: 2)
  btnBasic.borderWidth = 2;

  // setEventHandler(callback) — function called on touch
  // Callback signature: void handler(Button* btn)
  btnBasic.setEventHandler(&onBasic);

  // setDebounce(ms) — minimum time between touch events (default: 150ms)
  // Lower = more responsive but may double-trigger
  // Higher = more stable but feels sluggish
  btnBasic.setDebounce(200);

  canvas.add(&btnBasic, 4, 22);

  // --- Styled Button ---
  btnStyled.setColors(RED, WHITE, YELLOW);
  btnStyled.setSize(110, 36);
  btnStyled.fontSize = 1;
  btnStyled.setText("STYLED");

  // cornerRadius — rounds the button corners (default: 0 = square)
  // Uses fillRoundRect/drawRoundRect for rendering
  btnStyled.cornerRadius = 8;

  btnStyled.setEventHandler(&onStyled);
  canvas.add(&btnStyled, 122, 22);

  // --- Toggle Button ---
  btnToggle.setColors(GRAY2, WHITE, WHITE);
  btnToggle.setSize(232, 40);
  btnToggle.fontSize = 2;
  btnToggle.setText("OFF");

  // block — when true, touch event stops propagating to widgets below
  // Useful when buttons overlap or to prevent background widgets from firing
  btnToggle.block = true;

  btnToggle.setEventHandler(&onToggle);
  canvas.add(&btnToggle, 4, 66);

  // --- Terminal for feedback ---
  terminal.setColors(BLACK, GREEN, GRAY2);
  terminal.borderWidth = 1;
  canvas.add(&terminal, 4, 114);

  // --- Additional Properties (not used above, for reference) ---
  //
  // btn.isRound = true;
  //   Makes the button circular. Use Button(radius, bg, fg, border)
  //   constructor, or set manually. Draws with fillCircle/drawCircle.
  //
  // btn.labelPos = LABEL_LEFT;  // LABEL_LEFT(0), LABEL_TOP(1),
  //                              // LABEL_RIGHT(2), LABEL_BOTTOM(3)
  //   Position of external label relative to button.
  //
  // btn.setLabel("Label");
  //   Sets an external label drawn outside the button border.
  //
  // btn.fitToText();
  //   Auto-sizes the button (w, h) to fit the current text content.
  //
  // btn.touched
  //   Boolean that toggles on each press. Read in event handler
  //   to implement toggle/checkbox behavior.

  terminal.print("Ready. Tap a button.");
}

// ---- Loop ----
void loop() {
  // canvas.scan() must be called every loop iteration.
  // It polls touch input and dispatches events to widgets.
  canvas.scan();
}
