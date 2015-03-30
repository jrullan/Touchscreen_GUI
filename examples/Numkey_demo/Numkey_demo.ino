#include <SPI.h>
#include <Numkey.h>
#include <Dial.h>

Canvas canvas = Canvas();
Dial dial = Dial();
Numkey numkey = Numkey();
Button btnShow = Button(80,40,BLACK,GREEN,GRAY2);
Button btnInc = Button(40,40,GRAY1,BLACK,WHITE);
Button btnDec = Button(40,40,GRAY1,BLACK,WHITE);

void setup() {
  Serial.begin(9600);
  canvas.init(TFT_LANDSCAPE);
  
  dial.setSize(50);
  dial.setColors(BLACK,BLUE,WHITE);
  dial.setLimits(0,50,100);
  dial.init();
  dial.setHiLimit(75,GREEN);
  dial.setLowLimit(25,RED);
  
  btnShow.setText("Show");
  btnShow.init();
  btnShow.setEventHandler(&showEventHandler);

  btnInc.setText("+");
  btnInc.setEventHandler(&incValueEventHandler);
  btnInc.init();
  btnInc.setDebounce(200);

  btnDec.setText("-");
  btnDec.setEventHandler(&decValueEventHandler);
  btnDec.init();
  btnDec.setDebounce(200);

    
  numkey.setSize(120,160);
  numkey.setColors(GRAY1,BLACK,WHITE);
  //numkey.setNum(100);
  numkey.init();
  numkey.setNumkeyEventHandler(&numkeyEventHandler);
  
  canvas.add(&btnShow,0,0);
  canvas.add(&btnInc,250,10);
  canvas.add(&btnDec,250,50);
  canvas.add(&dial,numkey.w+20+dial.radius,dial.radius);
}

void loop() {
  canvas.scan();
}

// Event Handlers
//===================================
void numkeyEventHandler(Numkey* nk){
  if(*nk->getText()){
    dial.setCV(nk->getNum());
    btnShow.setNum(nk->getNum());
  }
  nk->clear();
  canvas.pop(); //Remove the keypad from the canvas
}

void showEventHandler(Button* btn){
  canvas.add(&numkey,0,0);
}

void incValueEventHandler(Button* btn){
  int inc = 5;
  int val = dial.getCV();
  if(val<100){
    dial.setCV(constrain(val+inc,0,100));
    btnShow.setNum(dial.getCV());
  }
}

void decValueEventHandler(Button* btn){
  int dec = 5;
  int val = dial.getCV();
  if(val>0){
    dial.setCV(constrain(val-dec,0,100));
    btnShow.setNum(dial.getCV());
  }
}

