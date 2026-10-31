// Optional on-device baseline. No networking credentials; compile does not flash.
#include <Canvas_XPT2046.h>
#include <Display.h>
#include <Button.h>
#include <Terminal.h>
#include <Trend.h>
#include <MultiTrend.h>
#include <Gauge.h>
#include <Dial.h>
Canvas_XPT2046 canvas(TFT_PORTRAIT, BLACK, 16, 15, 0);
Display* value = nullptr;
MultiTrend* chart = nullptr;
void heapReport(const char* stage) {
  Serial.print(stage);
#if defined(ESP8266)
  uint32_t freeBytes; uint16_t largest; uint8_t fragmentation;
  ESP.getHeapStats(&freeBytes,&largest,&fragmentation);
  Serial.print(" free_heap="); Serial.print(freeBytes);
  Serial.print(" largest_block="); Serial.print(largest);
  Serial.print(" fragmentation_percent="); Serial.print(fragmentation);
  Serial.print(" free_cont_stack="); Serial.print(ESP.getFreeContStack());
#endif
  Serial.println();
}
void setup() {
 Serial.begin(115200);
 Serial.print("sizeof Widget=");Serial.println(sizeof(Widget));
 Serial.print("sizeof Display=");Serial.println(sizeof(Display));
 Serial.print("sizeof Button=");Serial.println(sizeof(Button));
 Serial.print("sizeof Terminal=");Serial.println(sizeof(Terminal));
 Serial.print("sizeof Trend=");Serial.println(sizeof(Trend));
 Serial.print("sizeof MultiTrend=");Serial.println(sizeof(MultiTrend));
 Serial.print("sizeof Canvas_XPT2046=");Serial.println(sizeof(Canvas_XPT2046));
 heapReport("before_widgets");
 value=new Display(100,20,BLACK,WHITE,WHITE,12);
 chart=new MultiTrend(220,150,0,50,100,3,32);
 canvas.add(value,10,10,false);canvas.add(chart,10,40,false);
 char initial[]="Baseline";value->setText(initial);value->show();chart->show();
 heapReport("after_widgets");
}
void loop(){
 static unsigned long last=0; static uint8_t sample=0;
 const unsigned long now=millis();
 if(now-last>=1000){
  last=now;uint32_t start=micros();
  for(uint8_t s=0;s<3;++s)chart->addValue(s,(sample+s*25)%101,false);
  chart->update();
  Serial.print("chart_update_us=");Serial.println(micros()-start);
  heapReport("steady_state");++sample;
 }
 canvas.scan();yield();
}
