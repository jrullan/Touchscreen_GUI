#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Keyboard.h>
#include <Terminal.h>
#include <Trend.h>
#include <MultiTrend.h>

// Compatibility fixture, not an automatic upload or MCU timing benchmark.
TouchDriver_XPT2046 driver(0);
TouchMapper mapper(&driver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, &mapper, 16, 15);
Keyboard keyboard(Button::TextStorage::None);
Terminal terminal(230, 40, TERMINAL_SCROLL_DOWN, 1, Terminal::ScrollStorage::RotateRows);
Trend trend(110, 60, 0, 50, 100, 32, HistoryStorage::Circular);
MultiTrend multi(110, 60, 0, 50, 100, 3, 32, 3, HistoryStorage::Circular);
void setup() {
  Serial.begin(115200);
  canvas.init();
  if (!canvas.isReady()) return;
  canvas.add(&terminal, 4, 4, false);
  canvas.add(&trend, 4, 50, false);
  canvas.add(&multi, 120, 50, false);
  canvas.add(&keyboard, 0, 128, false);
  for (unsigned int i=0; i<80; ++i) {
    trend.addValue(i%101, false);
    multi.addValue(i%3, i%101, false);
  }
  terminal.print("Rotating rows");
  terminal.scrollUp(); terminal.scrollDown();
  keyboard.setText("Mode test");
  canvas.showWidgets();
  Serial.printf("OPTIMIZATIONS trend_latest=%u multi_latest=%u\n", trend.sample(0), multi.sample(0,0));
}
void loop() { if (canvas.isReady()) canvas.scan(); yield(); }
