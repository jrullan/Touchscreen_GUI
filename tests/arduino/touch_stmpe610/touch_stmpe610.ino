// Compile-only fixture: no attached touch_stmpe610 hardware is claimed.
#include <Canvas_Touch.h>
#include <TouchDriver_STMPE610.h>
TouchDriver_STMPE610 driver(0);
TouchMapper mapper(&driver, TouchCal::STMPE610_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, &mapper, 16, 15);
void setup(){ Serial.begin(115200); canvas.init(); }
void loop(){ if(canvas.isReady()) canvas.scan(); yield(); }
