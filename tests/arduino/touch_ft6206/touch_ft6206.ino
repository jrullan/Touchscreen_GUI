// Compile-only fixture: no attached touch_ft6206 hardware is claimed.
#include <Canvas_Touch.h>
#include <TouchDriver_FT6206.h>
TouchDriver_FT6206 driver;
TouchMapper mapper(&driver, TouchCal::FT6206_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, &mapper, 16, 15);
void setup(){ Serial.begin(115200); canvas.init(); }
void loop(){ if(canvas.isReady()) canvas.scan(); yield(); }
