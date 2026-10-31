#include <Canvas_XPT2046.h>
Canvas_XPT2046 canvas(TFT_PORTRAIT, BLACK, 16, 15, 0);
#include <Button.h>
Button button(80,30,BLACK,WHITE,WHITE);
void pressed(Button*){}
void setup(){button.setEventHandler(pressed); canvas.add(&button,8,8,false); button.setText("Press"); button.show();}
void loop(){canvas.scan();}
