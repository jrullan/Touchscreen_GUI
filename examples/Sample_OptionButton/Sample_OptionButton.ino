// ============================================================
// Sample_OptionButton — OptionButton Widget Reference
// ============================================================
// Demonstrates the OptionButton (toggle/radio) widget:
//   - Circular toggle buttons
//   - Event handler tracks selection state
//   - Terminal shows selection feedback
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <OptionButton.h>
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

// OptionButton() — default constructor
// Creates a circular button that toggles on each press.
// Inherits from Button, so shares setText(), setEventHandler(), etc.
OptionButton opt1 = OptionButton();
OptionButton opt2 = OptionButton();
OptionButton opt3 = OptionButton();

Terminal terminal = Terminal(232, 100, TERMINAL_SCROLL_UP, 1);

// ---- Event Handlers ----
// OptionButton event handlers work just like Button handlers.
// btn->touched toggles on each press.

void onOption1(Button* btn) {
  terminal.print(btn->touched ? "Option 1: ON" : "Option 1: OFF",
                 btn->touched ? GREEN : RED);
}

void onOption2(Button* btn) {
  terminal.print(btn->touched ? "Option 2: ON" : "Option 2: OFF",
                 btn->touched ? GREEN : RED);
}

void onOption3(Button* btn) {
  terminal.print(btn->touched ? "Option 3: ON" : "Option 3: OFF",
                 btn->touched ? GREEN : RED);
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
  tft->print("Sample: OptionButton Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // OPTIONBUTTON CONFIGURATION REFERENCE
  // ============================================================

  // OptionButton inherits from Button.
  // It renders as a circular button (isRound = true).
  // All Button properties apply: setColors(), setEventHandler(), etc.

  // --- Option 1 ---
  // setColors(bgColor, fgColor, borderColor)
  opt1.setColors(BLACK, WHITE, WHITE);

  // setSize(width, height) — sets the button dimensions
  // For round buttons, the diameter is min(width, height)
  opt1.setSize(30, 30);

  // fontSize — text size inside the button
  opt1.fontSize = 1;

  // setText() — label text inside the circle
  opt1.setText("1");

  // labelPos — position of external label
  //   LABEL_LEFT(0), LABEL_TOP(1), LABEL_RIGHT(2), LABEL_BOTTOM(3)
  opt1.labelPos = LABEL_RIGHT;

  // setLabel() — external label drawn outside the button
  opt1.setLabel((char*)"Speed");

  opt1.setEventHandler(&onOption1);
  canvas.add(&opt1, 30, 40);

  // --- Option 2 ---
  opt2.setColors(BLACK, WHITE, WHITE);
  opt2.setSize(30, 30);
  opt2.fontSize = 1;
  opt2.setText("2");
  opt2.labelPos = LABEL_RIGHT;
  opt2.setLabel((char*)"Power");
  opt2.setEventHandler(&onOption2);
  canvas.add(&opt2, 30, 80);

  // --- Option 3 ---
  opt3.setColors(BLACK, WHITE, WHITE);
  opt3.setSize(30, 30);
  opt3.fontSize = 1;
  opt3.setText("3");
  opt3.labelPos = LABEL_RIGHT;
  opt3.setLabel((char*)"Mode");
  opt3.setEventHandler(&onOption3);
  canvas.add(&opt3, 30, 120);

  // --- Terminal for feedback ---
  terminal.setColors(BLACK, GREEN, GRAY2);
  terminal.borderWidth = 1;
  canvas.add(&terminal, 4, 168);

  // --- Additional Properties (for reference) ---
  //
  // OptionButton is essentially a round Button with toggle behavior.
  // The framework toggles btn->touched on each press automatically.
  //
  // For mutually exclusive (radio) behavior, implement it in your
  // event handlers by resetting other buttons' touched state:
  //
  //   void onOption(Button* btn) {
  //     opt1.touched = false; opt2.touched = false; opt3.touched = false;
  //     btn->touched = true;
  //     // Redraw all options...
  //   }

  terminal.print("Tap an option to toggle.", YELLOW);
}

// ---- Loop ----
void loop() {
  canvas.scan();
}
