#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Keyboard.h>
#include <IconButton.h>

// Compile fixture only: no credentials and no automatic device upload.
TouchDriver_XPT2046 driver(0);
TouchMapper mapper(&driver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, &mapper, 16, 15);
const uint16_t image[] PROGMEM = {0xffff};
Button target(232, 40, BLUE, WHITE, WHITE, 48);
Keyboard legacyKeyboard;
Keyboard leanKeyboard(Button::TextStorage::None);
Keyboard styledKeyboard(240, 192, BLACK, WHITE, RED, Button::TextStorage::None);
IconButton legacyIcon(1, 1, image);
IconButton leanIcon(1, 1, image, nullptr, nullptr, Button::TextStorage::None);
void setup() {
  Serial.begin(115200);
  canvas.init();
  if (!canvas.isReady()) return;
  canvas.add(&target, 4, 8, false);
  canvas.add(&leanKeyboard, 0, 128, false);
  canvas.add(&leanIcon, 4, 60, false);
  target.setText("Edit me");
  leanKeyboard.setText("Independent buffer");
  Button* base = &leanKeyboard;
  base->setText("safe no-op");
  base->fitToText();
  leanKeyboard.show(&target);
  leanIcon.show();
  Serial.printf("OPTIONAL_TEXT legacy=%d lean=%d keyboard_size=%u icon_size=%u\n",
    legacyKeyboard.contents != nullptr && legacyIcon.contents != nullptr,
    leanKeyboard.contents == nullptr && styledKeyboard.contents == nullptr && leanIcon.contents == nullptr,
    unsigned(sizeof(Keyboard)), unsigned(sizeof(IconButton)));
}
void loop() { if (canvas.isReady()) canvas.scan(); yield(); }
