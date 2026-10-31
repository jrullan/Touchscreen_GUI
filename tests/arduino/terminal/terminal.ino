#include <Canvas_XPT2046.h>
Canvas_XPT2046 canvas(TFT_PORTRAIT, BLACK, 16, 15, 0);
#include <Terminal.h>
Terminal terminal(200,100);
void setup(){canvas.add(&terminal,8,8,false); terminal.show(); terminal.print("Ready");}
void loop(){canvas.scan();}
