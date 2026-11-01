#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Keyboard.h>

// Compile fixture and standalone physical demo. No network credentials.
TouchDriver_XPT2046 driver(0);
TouchMapper mapper(&driver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, &mapper, 16, 15);
Button target(232, 40, BLUE, WHITE, WHITE, 48);
Keyboard keyboard;
void completed(Keyboard* k) {
  Serial.printf("KEYBOARD result=%s length=%u text=%s target=%s heap=%u\n",
    k->result() == Keyboard::Accepted ? "accepted" : "cancelled",
    k->getTextSize(), k->getText(), target.getText(), ESP.getFreeHeap());
}
void edit(Button*) { keyboard.show(&target); }
void setup() {
  Serial.begin(115200);
  canvas.init();
  if (!canvas.isReady()) { Serial.println("KEYBOARD startup failed"); return; }
  canvas.add(&target, 4, 8, false);
  target.setText("Edit me"); target.fontSize = 1;
  target.setEventHandler(edit);
  canvas.add(&keyboard, 0, 128, false);
  keyboard.setEventHandler(completed);
  canvas.showWidgets();
  keyboard.show(&target);
  Serial.printf("KEYBOARD ready sizeof=%u heap=%u\n", unsigned(sizeof(Keyboard)), ESP.getFreeHeap());
}
void loop() { if (canvas.isReady()) canvas.scan(); yield(); }
