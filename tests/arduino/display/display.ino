#include <Canvas_XPT2046.h>
Canvas_XPT2046 canvas(TFT_PORTRAIT, BLACK, 16, 15, 0);
#include <Display.h>
Display value(80,20,BLACK,WHITE,WHITE,12);
void setup(){canvas.add(&value,8,8,false); char text[]="47 W"; value.setText(text); value.show();}
void loop(){canvas.scan();}
