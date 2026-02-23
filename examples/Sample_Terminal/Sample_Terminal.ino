// ============================================================
// Sample_Terminal — Terminal Widget Reference
// ============================================================
// Demonstrates the Terminal (scrolling text) widget:
//   - Multi-line scrolling output with per-line colors
//   - print(), printf() with formatting
//   - Scroll direction toggle via button
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Terminal.h>
#include <Button.h>
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

// Terminal(width, height, direction, fontSize)
//   width:     terminal width in pixels
//   height:    terminal height in pixels
//   direction: TERMINAL_SCROLL_UP or TERMINAL_SCROLL_DOWN
//   fontSize:  text size multiplier (default: 1)
Terminal terminal = Terminal(232, 180, TERMINAL_SCROLL_UP, 1);

Button btnClear  = Button();
Button btnScroll = Button();

// ---- Timers ----
Neotimer msgTimer = Neotimer(1500);
uint16_t lineNum = 0;

// ---- Event Handlers ----
void onClear(Button* btn) {
  // clear() — removes all text from the terminal
  terminal.clear();
  terminal.print("Terminal cleared.", YELLOW);
  lineNum = 0;
}

void onScroll(Button* btn) {
  // direction — toggle between TERMINAL_SCROLL_UP and TERMINAL_SCROLL_DOWN
  if (terminal.direction == TERMINAL_SCROLL_UP) {
    terminal.direction = TERMINAL_SCROLL_DOWN;
    btn->setText("DIR: DOWN");
    terminal.print("Scroll: DOWN", CYAN);
  } else {
    terminal.direction = TERMINAL_SCROLL_UP;
    btn->setText("DIR: UP");
    terminal.print("Scroll: UP", CYAN);
  }
  btn->show();
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
  tft->print("Sample: Terminal Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // TERMINAL CONFIGURATION REFERENCE
  // ============================================================

  // setColors(bgColor, fgColor, borderColor)
  //   bgColor:     terminal background
  //   fgColor:     default text color (used when no highlight color specified)
  //   borderColor: border color
  terminal.setColors(BLACK, GREEN, GRAY2);

  // borderWidth — border thickness (default: 2)
  terminal.borderWidth = 1;

  // direction — scroll direction
  //   TERMINAL_SCROLL_UP:   new lines appear at bottom, scroll upward
  //   TERMINAL_SCROLL_DOWN: new lines appear at top, scroll downward

  // lineSpace — extra pixels between lines (default: 0)
  // Increase for more readable spacing at the cost of fewer visible lines
  terminal.lineSpace = 1;

  // keepColors — if true, each line retains its original highlight color
  // after scrolling. If false, all lines use fgColor. (default: 0)
  terminal.keepColors = 1;

  // lines — number of visible text lines (auto-calculated from height/fontSize)
  // Read-only. Maximum: MAX_LINES (10).

  // useHardwareScroll — use ILI9341 hardware scroll (portrait mode only)
  // Faster scrolling but only works in portrait orientation.
  // terminal.useHardwareScroll = true;

  canvas.add(&terminal, 4, 18);

  // --- Buttons ---
  btnClear.setColors(GRAY2, WHITE, WHITE);
  btnClear.setSize(112, 28);
  btnClear.fontSize = 1;
  btnClear.setText("CLEAR");
  btnClear.setEventHandler(&onClear);
  canvas.add(&btnClear, 4, 206);

  btnScroll.setColors(GRAY2, WHITE, WHITE);
  btnScroll.setSize(112, 28);
  btnScroll.fontSize = 1;
  btnScroll.setText("DIR: UP");
  btnScroll.setEventHandler(&onScroll);
  canvas.add(&btnScroll, 124, 206);

  // --- Boot messages ---
  // print(text, highlightColor)
  //   text:           char*, const char*, or String
  //   highlightColor: optional per-line color (0 = use fgColor)
  terminal.print("Terminal initialized.", YELLOW);
  terminal.print("Lines auto-scroll when full.");
  terminal.print("Each line can have its own color.");

  // --- Additional Methods (for reference) ---
  //
  // terminal.printf(format, number, highlightColor)
  //   Prints a formatted string with one %d substitution.
  //   Example: terminal.printf("Value: %d", 42, GREEN);
  //
  // terminal.print(String str, highlightColor)
  //   Accepts Arduino String objects as well as char*/const char*.
  //
  // terminal.scroll()
  //   Manually triggers a scroll in the configured direction.
  //
  // terminal.clear()
  //   Clears all lines and resets the terminal.
  //
  // terminal.linesDirty[i]
  //   Per-line dirty flags. update() skips clean lines.
  //
  // terminal.maxCharacters
  //   Max characters per line (auto-calculated from width/fontSize).

  // Labels
  tft->setTextColor(GRAY1);
  tft->setTextSize(1);
  tft->setCursor(4, 242);
  tft->print("Messages print every 1.5 sec.");

  msgTimer.start();
}

// ---- Loop ----
void loop() {
  canvas.scan();

  if (msgTimer.repeat()) {
    lineNum++;

    // Alternate between print() and printf() to demonstrate both
    if (lineNum % 3 == 0) {
      // printf() — format string with one %d substitution
      terminal.printf((char*)"Line %d [formatted]", lineNum, CYAN);
    } else {
      char buf[32];
      sprintf(buf, "Line %d", lineNum);
      terminal.print(buf);
    }
  }
}
