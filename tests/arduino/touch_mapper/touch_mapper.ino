#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
// Dependencies outlive their borrowers. Existing Canvas_XPT2046 stays supported.
TouchDriver_XPT2046 driver(0);
TouchMapper mapper(&driver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, &mapper, 16, 15);
void setup(){
 Serial.begin(115200);
 canvas.init();
 Serial.printf("TOUCH_BEGIN ready=%d\n", canvas.isReady());
 for(uint8_t rotation=0;rotation<4;++rotation){
  canvas.setRotation(rotation);
  Serial.printf("rotation=%u width=%d height=%d\n",rotation,canvas.w,canvas.h);
 }
 canvas.portrait();
 if(canvas.isReady()){
  canvas.tft->setTextColor(WHITE);canvas.tft->setTextSize(2);
  canvas.tft->setCursor(12,12);canvas.tft->print("Touch mapper");
 }
}
void loop(){
 if(!canvas.isReady()){delay(50);return;}
 Point* point=canvas.getTouchedPoint();
 if(point){
  canvas.tft->fillCircle(point->x,point->y,2,GREEN);
  Serial.printf("point x=%d y=%d z=%d\n",point->x,point->y,point->z);
 }
 yield();
}
