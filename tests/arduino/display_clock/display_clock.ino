#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Display.h>
#include <stdio.h>

// Clock demo and target-compile fixture. No network or credentials.
TouchDriver_XPT2046 driver(0);
TouchMapper mapper(&driver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, &mapper, 16, 15);
Display clockDisplay(220, 48, BLACK, WHITE, GREEN, 8);
unsigned long previousSecond = 0;
bool clockReady = false;
void refreshClock(unsigned long seconds) {
  char text[9];
  const unsigned int h = (seconds / 3600UL) % 24UL;
  const unsigned int m = (seconds / 60UL) % 60UL;
  const unsigned int s = seconds % 60UL;
  snprintf(text, sizeof(text), "%02u:%02u:%02u", h, m, s);
  // setText(...,true) draws only changes when eligible; update() forces repair.
  clockDisplay.setText(text, true);
}
void setup() {
  Serial.begin(115200);
  canvas.init();
  if (!canvas.isReady()) { Serial.println("DISPLAY_CLOCK startup failed"); return; }
  canvas.add(&clockDisplay, 10, 120, false);
  clockDisplay.fontSize = 3;
  clockDisplay.borderWidth = 1;
  clockDisplay.show();
  refreshClock(0);
  clockReady = true;
  Serial.println("DISPLAY_CLOCK ready; elapsed HH:MM:SS");
}
void loop() {
  if (!clockReady) { yield(); return; }
  const unsigned long seconds = millis() / 1000UL;
  if (seconds != previousSecond) {
    previousSecond = seconds;
    refreshClock(seconds);
  }
  canvas.scan();
  yield();
}
