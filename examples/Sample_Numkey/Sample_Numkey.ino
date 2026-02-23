// ============================================================
// Sample_Numkey — Numkey Widget Reference
// ============================================================
// Demonstrates the Numkey (on-screen numeric keypad) widget:
//   - Triggered by button press
//   - Number entry with display feedback
//   - Auto-remove from canvas on confirm
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Numkey.h>
#include <Display.h>
#include <Button.h>

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

// Numkey() — default constructor
// Creates a full numeric keypad (0-9, ., DEL, =)
// Default size: 120x160
Numkey numkey = Numkey();

// Display to show the entered value
Display dispResult = Display(12);
Display dispLabel  = Display(20);

// Button to trigger the numkey
Button btnEnter = Button();

// ---- Forward declarations ----
void onEnter(Button* btn);
void onNumkeyDone(Numkey* nk);

// ---- Event Handlers ----

// Button handler: shows the numkey when not already visible
void onEnter(Button* btn) {
  numkey.show(btn);
}

// Numkey event handler — called when "=" is pressed
void onNumkeyDone(Numkey* nk) {
  Serial.print("Numkey entry: ");
  Serial.println(nk->getNum());
  dispResult.setText(nk->getText());
  dispResult.update();
  nk->clear();
  nk->hide();
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
  tft->print("Sample: Numkey Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // NUMKEY CONFIGURATION REFERENCE
  // ============================================================

  // setColors(bgColor, fgColor, borderColor)
  //   bgColor:     keypad background
  //   fgColor:     key text color
  //   borderColor: key border color
  numkey.setColors(GRAY2, WHITE, WHITE);

  // fontSize — key label font size
  numkey.fontSize = 2;

  // autoremove — if true, numkey removes itself from canvas when "=" is pressed
  // When false, you must manually call hide() and pop() to remove it.
  numkey.autoremove = true;

  numkey.init(); // Initializes internal state (must be called before use)

  // setSize(width, height) — keypad dimensions
  // Includes display area at top + 4x3 key grid
  numkey.setSize(120, 160);  


  // setEventHandler(callback) — called when "=" is pressed
  // Callback signature: void handler(Numkey* nk)
  numkey.setEventHandler(&onNumkeyDone);
  canvas.add(&numkey, 60, 80);

  // --- Result Display ---
  dispLabel.setColors(BLACK, GRAY1, BLACK);
  dispLabel.fontSize = 1;
  dispLabel.borderWidth = 0;
  dispLabel.setSize(120, 12);
  dispLabel.setText("ENTERED VALUE:");
  canvas.add(&dispLabel, 60, 22);

  dispResult.setColors(BLACK, GREEN, GRAY2);
  dispResult.fontSize = 3;
  dispResult.borderWidth = 1;
  dispResult.setSize(120, 36);
  dispResult.setText("---");
  canvas.add(&dispResult, 60, 38);

  // --- Trigger Button ---
  btnEnter.setColors(BLUE, WHITE, WHITE);
  btnEnter.setSize(120, 36);
  btnEnter.fontSize = 2;
  btnEnter.setText("ENTER #");
  btnEnter.setEventHandler(&onEnter);
  canvas.add(&btnEnter, 60, 270);

  // --- Additional Properties (for reference) ---
  //
  // numkey.getNum() — returns the entered number as a long integer
  // numkey.getText() — returns the entered text as char*
  // numkey.getTextSize() — returns the number of characters entered
  //
  // numkey.clear() — clears the current entry
  // numkey.deleteChar() — removes the last character (DEL key)
  // numkey.append("text") — appends text to the entry
  //
  // numkey.show(Button* btn) — shows numkey and sets target button
  //   When "=" is pressed, the target button's event handler is called.
  //   This creates a two-step interaction:
  //   1. First press: show numkey
  //   2. Entry confirmed: button handler runs again to process value
  //
  // numkey.hide() — hides numkey and redraws overlapped widgets
  // numkey.reset() — clears entry state for next use
  //
  // numkey.targetButton — pointer to the linked button
  // numkey.entry — entry state flag

  // Labels
  tft->setTextColor(GRAY1);
  tft->setTextSize(1);
  tft->setCursor(4, 252);
  tft->print("Tap ENTER to show numeric keypad.");
}

// ---- Loop ----
void loop() {
  canvas.scan();
}
