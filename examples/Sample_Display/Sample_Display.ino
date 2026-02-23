// ============================================================
// Sample_Display — Display Widget Reference
// ============================================================
// Demonstrates the Display widget for showing text and numbers:
//   - Text display with custom colors
//   - Numeric display updated by timer
//   - Different font sizes and border styles
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Display.h>
#include <neotimer.h>

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

// Display(textLength) — text buffer size (default: 8)
// Buffer auto-grows via realloc if setText() receives longer text.
Display dispTitle  = Display(20);
Display dispValue  = Display(8);
Display dispSmall  = Display(16);
Display dispNoBorder = Display(24);

// ---- Timers ----
Neotimer valueTimer = Neotimer(500);
int counter = 0;

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  canvas.init();
  Guitft* tft = canvas.tft;

  // Title
  tft->setTextColor(WHITE);
  tft->setTextSize(1);
  tft->setCursor(4, 4);
  tft->print("Sample: Display Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // DISPLAY CONFIGURATION REFERENCE
  // ============================================================

  // --- Title Display (large text, no border) ---
  // setColors(bgColor, fgColor, borderColor)
  dispTitle.setColors(BLACK, CYAN, BLACK);

  // fontSize — text size multiplier (1=6x8, 2=12x16, etc.)
  dispTitle.fontSize = 2;

  // borderWidth — border thickness (set to 0 to hide border)
  dispTitle.borderWidth = 0;

  // setSize(width, height) — display area dimensions
  dispTitle.setSize(232, 20);

  // setText(text) — sets the display content
  // Only redraws if text actually changed (dirty flag optimization)
  dispTitle.setText("DISPLAY DEMO");

  canvas.add(&dispTitle, 4, 22);

  // --- Numeric Value Display (large, bordered) ---
  dispValue.setColors(BLUE, WHITE, CYAN);
  dispValue.fontSize = 3;
  dispValue.borderWidth = 2;
  dispValue.setSize(120, 40);
  dispValue.setText("0");
  canvas.add(&dispValue, 60, 56);

  // --- Small Info Display ---
  dispSmall.setColors(GRAY2, GREEN, GREEN);
  dispSmall.fontSize = 1;
  dispSmall.borderWidth = 1;
  dispSmall.setSize(232, 16);
  dispSmall.setText("Counter updates every 500ms");
  canvas.add(&dispSmall, 4, 110);

  // --- No Border Display ---
  // Setting borderWidth=0 removes the border entirely.
  // Useful for status bars and inline text.
  dispNoBorder.setColors(BLACK, YELLOW, BLACK);
  dispNoBorder.fontSize = 1;
  dispNoBorder.borderWidth = 0;
  dispNoBorder.setSize(232, 12);
  dispNoBorder.setText("borderWidth=0 (no border)");
  canvas.add(&dispNoBorder, 4, 138);

  // --- Additional Properties (not used above, for reference) ---
  //
  // display.append("text");
  //   Appends text to the current content (like typing).
  //
  // display.append('c');
  //   Appends a single character.
  //
  // display.deleteChar();
  //   Removes the last character (like backspace).
  //
  // display.fitToText();
  //   Auto-sizes the display to fit the current text content.
  //
  // Display also accepts setText(const char*) for string literals.

  // Labels
  tft->setTextColor(GRAY1);
  tft->setTextSize(1);
  tft->setCursor(4, 166);
  tft->print("Display is read-only (no touch).");
  tft->setCursor(4, 178);
  tft->print("Use setText() to update content.");
  tft->setCursor(4, 190);
  tft->print("Only redraws when text changes.");

  valueTimer.start();
}

// ---- Loop ----
void loop() {
  canvas.scan();

  if (valueTimer.repeat()) {
    counter++;
    char buf[8];
    sprintf(buf, "%d", counter);

    // setText() marks the widget dirty; update() redraws only if dirty
    dispValue.setText(buf);
    dispValue.update();
  }
}
