// ============================================================
// Sample_Keyboard — QWERTY Keyboard Widget Reference
// ============================================================
// Demonstrates the Keyboard (on-screen QWERTY keypad) widget:
//   - Triggered by button press
//   - Text entry with display feedback
//   - Shift/uppercase and special character modes
//   - ENTER key fires event handler
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Keyboard.h>
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
Keyboard keyboard = Keyboard();

Display dispLabel  = Display(20);
Display dispResult = Display(KEYBOARD_BUF_SIZE);

Button btnType = Button();

// ---- Forward declarations ----
void onType(Button* btn);
void onKeyboardDone(Keyboard* kb);

// ---- Event Handlers ----

// Button handler: shows the keyboard
void onType(Button* btn) {
  if(!keyboard.visible) {
    canvas.add(&keyboard, 0, 140);
    keyboard.show(btn);
  }
}

// Keyboard ENTER handler
void onKeyboardDone(Keyboard* kb) {
  Serial.print("Entered: ");
  Serial.println(kb->getText());

  dispResult.setText(kb->getText());
  dispResult.update();

  kb->hide();
  kb->reset();
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
  tft->print("Sample: Keyboard Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // KEYBOARD CONFIGURATION REFERENCE
  // ============================================================

  // init() — must be called before customization
  keyboard.init();

  // setColors(bgColor, fgColor, borderColor)
  keyboard.setColors(GRAY2, WHITE, WHITE);

  // setSize(width, height) — keyboard dimensions
  // Height includes: display row + 4 character rows + bottom row (SPACE/ENTER)
  keyboard.setSize(240, 180);

  // fontSize — key label font size (1 recommended for 240px width)
  keyboard.fontSize = 1;

  // autoremove — if true, keyboard pops itself from canvas on ENTER
  keyboard.autoremove = false;

  // setEventHandler(callback) — called when ENTER/OK is pressed
  // Callback signature: void handler(Keyboard* kb)
  keyboard.setEventHandler(&onKeyboardDone);

  // --- Result Display ---
  dispLabel.setColors(BLACK, GRAY1, BLACK);
  dispLabel.fontSize = 1;
  dispLabel.borderWidth = 0;
  dispLabel.setSize(200, 12);
  dispLabel.setText("ENTERED TEXT:");
  canvas.add(&dispLabel, 4, 22);

  dispResult.setColors(BLACK, GREEN, GRAY2);
  dispResult.fontSize = 2;
  dispResult.borderWidth = 1;
  dispResult.setSize(232, 30);
  dispResult.setText("---");
  canvas.add(&dispResult, 4, 38);

  // --- Trigger Button ---
  btnType.init();
  btnType.setColors(BLUE, WHITE, WHITE);
  btnType.setSize(232, 36);
  btnType.fontSize = 2;
  btnType.setText("TYPE");
  btnType.setEventHandler(&onType);
  canvas.add(&btnType, 4, 80);

  // --- Help text ---
  tft->setTextColor(GRAY1);
  tft->setTextSize(1);
  tft->setCursor(4, 124);
  tft->print("Tap TYPE to show keyboard.");
  tft->setCursor(4, 134);
  tft->print("Sh=Shift  !#=Special  OK=Enter");
}

// ---- Loop ----
void loop() {
  canvas.scan();
}
