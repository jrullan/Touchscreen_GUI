// Identical before/after widget-render workload. No WiFi or credentials.
#include <Canvas_XPT2046.h>
#include <Gauge.h>
#include <Slider.h>
#include <Terminal.h>
Canvas_XPT2046 canvas(TFT_PORTRAIT,BLACK,16,15,0);
Gauge gauge(50,120,0,50,100);
Slider slider;
Terminal terminal(220,60,TERMINAL_SCROLL_UP,1);
void setup(){
 Serial.begin(115200);delay(300);canvas.scan();
 slider.setSize(40,120);slider.setCV(50,false);
 canvas.add(&gauge,10,20,false);canvas.add(&slider,160,20,false);canvas.add(&terminal,10,210,false);
 gauge.show();slider.show();terminal.show();
 Serial.println("WIDGET_DELTA_BENCH_BEGIN");
 for(int i=0;i<80;++i){
  int v=i<=50?i*2:(80-i)*2;
  gauge.setCV(v,false);uint32_t t=micros();gauge.update();uint32_t gu=micros()-t;
  slider.setCV(v,false);t=micros();slider.update();uint32_t su=micros()-t;
  Serial.printf("changed i=%d gauge_us=%u slider_us=%u\n",i,gu,su);yield();delay(2);
 }
 for(int i=0;i<40;++i){
  uint32_t t=micros();gauge.update();uint32_t gu=micros()-t;
  t=micros();slider.update();uint32_t su=micros()-t;
  Serial.printf("unchanged i=%d gauge_us=%u slider_us=%u\n",i,gu,su);yield();
 }
 for(int i=0;i<20;++i){
  char line[24];snprintf(line,sizeof(line),"Sample %d",i);
  uint32_t t=micros();terminal.print(line);uint32_t tu=micros()-t;
  Serial.printf("terminal_print i=%d us=%u\n",i,tu);yield();
 }
 for(int i=0;i<20;++i){
  uint32_t t=micros();terminal.update();uint32_t tu=micros()-t;
  Serial.printf("terminal_unchanged i=%d us=%u\n",i,tu);yield();
 }
 Serial.printf("heap=%u largest=%u sizeofGauge=%u sizeofSlider=%u sizeofTerminal=%u\n",ESP.getFreeHeap(),ESP.getMaxFreeBlockSize(),sizeof(Gauge),sizeof(Slider),sizeof(Terminal));
 Serial.println("WIDGET_DELTA_BENCH_END");
}
void loop(){yield();}
