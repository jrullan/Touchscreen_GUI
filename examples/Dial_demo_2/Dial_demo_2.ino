// Required includes
#include <SPI.h>
#include <StandardCplusplus.h>
#include <vector>

// GUI includes
#include <Gauge.h>
#include <Dial.h>
#include <Button.h>
#include <Display.h>

// Create the objects
//==========================================
Canvas canvas = Canvas();
Button btnIncD = Button();
Button btnDecD = Button();
Dial dial = Dial();
Button btnIncV = Button();
Button btnDecV = Button();
Gauge value = Gauge();



void setup() {
  Serial.begin(9600);
  
  //Configure the objects
  //=========================================
    canvas.init(TFT_LANDSCAPE);
  
    //Prepare widgets
    dial.setSize(50);
    dial.setColors(GRAY2,YELLOW,GRAY1);
    dial.setLimits(0,50,100);
    dial.init();
    dial.setHiLimit(75,GREEN);
    dial.setLowLimit(25,RED);
    //dial.showVal = false;
    //dial.showTicks = false;

    btnIncD.setSize(40,40);
    btnIncD.setColors(GRAY1,BLACK,WHITE);
    btnIncD.setText("+");
    btnIncD.setEventHandler(&incDialEventHandler);
    btnIncD.init();
    btnIncD.setDebounce(200);
  
    btnDecD.setSize(40,40);
    btnDecD.setColors(GRAY1,BLACK,WHITE);
    btnDecD.setText("-");
    btnDecD.setEventHandler(&decDialEventHandler);
    btnDecD.init();
    btnDecD.setDebounce(200);
    
    canvas.add(&dial,5+dial.radius,30+dial.radius);
    canvas.add(&btnIncD,dial.x,dial.y+dial.radius+10);
    canvas.add(&btnDecD,dial.x-btnDecD.w,dial.y+dial.radius+10);   
    Tft.drawString("SP",dial.x-12,dial.y-dial.radius-16-2,2,WHITE);

    value.setSize(40,100);
    value.setColors(BLACK,BLUE,WHITE);
    value.setLimits(0,50,100);
    value.init();

    btnIncV.setSize(40,40);
    btnIncV.setColors(GRAY1,BLACK,WHITE);
    btnIncV.setText("+");
    btnIncV.setEventHandler(&incValueEventHandler);
    btnIncV.init();
    btnIncV.setDebounce(200);
  
    btnDecV.setSize(40,40);
    btnDecV.setColors(GRAY1,BLACK,WHITE);
    btnDecV.setText("-");
    btnDecV.setEventHandler(&decValueEventHandler);
    btnDecV.init();
    btnDecV.setDebounce(200);

    canvas.add(&value,50+dial.radius*2,30);
    canvas.add(&btnIncV,value.x+value.w,value.y+value.h+10);
    canvas.add(&btnDecV,value.x+value.w-btnDecV.w,value.y+value.h+10);
    Tft.drawString("CV",value.x+value.w-20,value.y-16-2,2,WHITE);
}

void loop() {
  canvas.scan();

}

// Event Handler Functions for buttons
//==========================================
void incDialEventHandler(Button* btn){
  int inc = 5;
  int val = dial.getCV();
  if(val==100) return;
  dial.setCV(val + inc);
}

void decDialEventHandler(Button* btn){
  int dec = 5;
  int val = dial.getCV();
  if(val==0) return;
  dial.setCV(val-dec);
}

void incValueEventHandler(Button* btn){
  int inc = 5;
  int val = value.getCV();
  if(val==100) return;
  value.setCV(val + inc);
  Tft.fillRectangle(200,12,3*6*2,8*2,BLACK);
  Tft.drawNumber(value.getCV(),200,12,2,BLUE);
}

void decValueEventHandler(Button* btn){
  int dec = 5;
  int val = value.getCV();
  if(val==0) return;
  value.setCV(val-dec);
  Tft.fillRectangle(200,12,3*6*2,8*2,BLACK);
  Tft.drawNumber(value.getCV(),200,12,2,BLUE);
}
