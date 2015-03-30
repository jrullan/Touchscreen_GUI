#include <SPI.h>
//These are included to use C++ vector class
#include <StandardCplusplus.h>
#include <vector>
#include <Canvas.h>
#include <Button.h>
#include <Display.h>

TouchScreen ts = TouchScreen(XP, YP, XM, YM);
Canvas canvas = Canvas();
Button btn1 = Button(50,50,GRAY1,BLACK,WHITE);
Button btn2 = Button(50,50,GRAY1,BLACK,WHITE);
Button btn3 = Button(50,50,GRAY1,BLACK,WHITE);
Button btn4 = Button(50,50,GRAY1,BLACK,WHITE);
Button btn5 = Button(50,50,GRAY1,BLACK,WHITE);
Button btn6 = Button(50,50,GRAY1,BLACK,WHITE);
Button btn7 = Button(50,50,GRAY1,BLACK,WHITE);
Button btn8 = Button(50,50,GRAY1,BLACK,WHITE);
Button btn9 = Button(50,50,GRAY1,BLACK,WHITE);
Button btn0 = Button(50,50,GRAY1,BLACK,WHITE);
Button btnDot = Button(50,50,GRAY1,BLACK,WHITE);
Button btnBack = Button(40,40,GRAY1,BLACK,WHITE);
Button btnClear = Button(40,40,GRAY1,BLACK,WHITE);
Button btnRotate = Button(50,50,GRAY1,BLACK,WHITE);
//Button btnFrame = Button(240,240,BLACK,BLACK,WHITE);
Display dspNum = Display(150,40,BLUE,WHITE,WHITE);

unsigned long lastMillis;
unsigned int numPos = 0;
unsigned int CHARSIZE = 12;

void setup() {
  Serial.begin(9600);
  canvas.init();
  Tft.layoutMode = TFT_LANDSCAPE;
  
  //canvas.add(&btnFrame,10,0);
  
  btn1.setText("1");
  btn1.setEventHandler(&numEventHandler);
  canvas.add(&btn1,10,40);
  
  btn2.setText("2");
  btn2.setEventHandler(&numEventHandler);
  canvas.add(&btn2,60,40);
  
  btn3.setText("3");
  btn3.setEventHandler(&numEventHandler);
  canvas.add(&btn3,110,40);
  
  btn4.setText("4");
  btn4.setEventHandler(&numEventHandler);
  canvas.add(&btn4,10,90);
  
  btn5.setText("5");
  btn5.setEventHandler(&numEventHandler);
  canvas.add(&btn5,60,90);
  
  btn6.setText("6");
  btn6.setEventHandler(&numEventHandler);
  canvas.add(&btn6,110,90);

  btn7.setText("7");
  btn7.setEventHandler(&numEventHandler);
  canvas.add(&btn7,10,140);
  
  btn8.setText("8");
  btn8.setEventHandler(&numEventHandler);
  canvas.add(&btn8,60,140);
  
  btn9.setText("9");
  btn9.setEventHandler(&numEventHandler);
  canvas.add(&btn9,110,140);

  btn0.setText("0");
  btn0.setEventHandler(&numEventHandler);
  canvas.add(&btn0,60,190);

  btnDot.setText(".");
  btnDot.setEventHandler(&numEventHandler);
  canvas.add(&btnDot,10,190);

  btnRotate.setText("P");
  btnRotate.setEventHandler(&rotateEventHandler);
  canvas.add(&btnRotate,110,190);
  
  btnBack.setText("<-");
  btnBack.setEventHandler(&backEventHandler);
  canvas.add(&btnBack,160,0);

  btnClear.setText("C");
  btnClear.setEventHandler(&clearEventHandler);
  canvas.add(&btnClear,200,0);
  
  dspNum.setText("0");
  canvas.add(&dspNum,10,0);
  
  Tft.drawRectangle(10,0,230,240,WHITE);
  lastMillis = millis();
}

void loop() {
  if(millis()>lastMillis + 100){ //Debouncing of the touchscreen resistance
    Point tP = getTouchedPoint();
    canvas.touchWidgets(&tP); //Update all buttons on Canvas :)
    lastMillis=millis();
  }
  //delay(100); //Required for debouncing the touchscreen resistors.
}

void rotateEventHandler(Button* btn){
  if(Tft.layoutMode == TFT_PORTRAIT){
    Tft.layoutMode = TFT_LANDSCAPE;
    Serial.println("Changing to landscape");
    btn->setText("P");
    canvas.redraw();
  }else{
    Tft.layoutMode = TFT_PORTRAIT;
    Serial.println("Changing to portrait");
    btn->setText("L");
    canvas.redraw();
  }
  Tft.drawRectangle(10,0,230,240,WHITE);
  numPos = 0;
}

void numEventHandler(Button* btn){
  dspNum.setText(btn->getText());
  dspNum.show();
  /*
  if(numPos >= 160){
    numPos = 0;
    Tft.fillRectangle(0,0,160,40,BLACK);
  }
  Tft.drawString(btn->getText(),numPos+10,11,2,GREEN);
  numPos = numPos + CHARSIZE;*/
}

void backEventHandler(Button* btn){
  if(numPos > CHARSIZE+10){
    numPos = numPos - CHARSIZE;
    Tft.fillRectangle(numPos+1+10,2,CHARSIZE-1,37,BLACK);
    Serial.print("numPos = ");Serial.println(numPos);  
  }else{
    Tft.fillRectangle(12,2,numPos+CHARSIZE-1,37,BLACK);
    Serial.print("else, numPos = ");Serial.println(numPos);
    numPos = 0;
  }    
}

void clearEventHandler(Button* btn){
  Tft.fillRectangle(12,2,148,38,BLACK);
  numPos = 0;
}

Point getTouchedPoint(){
  Point p = ts.getPoint();
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);
  if(Tft.layoutMode == TFT_LANDSCAPE) p.toLandscape();
  return p;//toLandscape(&p);
}


/*
// Transpose a point to landscape mode
Point toLandscape(Point *p){
	Point tP;
	tP.x = 320 - p->y;
	tP.y = p->x;
	tP.z = p->z;
	return tP;
}

// Transpose a point to portrait mode
Point toPortrait(Point *p){
	Point tP;
	tP.y = 320 - p->x;
	tP.x = p->y;
	tP.z = p->z;
	return tP;
}
*/
