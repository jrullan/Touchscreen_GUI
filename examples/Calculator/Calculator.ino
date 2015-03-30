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
Button btnEqual = Button(80,40,GRAY1,BLACK,WHITE);
Button btnPlus = Button(80,40,GRAY1,BLACK,WHITE);
Button btnMinus = Button(80,40,GRAY1,BLACK,WHITE);
Button btnMult = Button(40,40,GRAY1,BLACK,WHITE);
Button btnDiv = Button(40,40,GRAY1,BLACK,WHITE);

Display dspNum = Display(120,40,BLACK,GREEN,GRAY1);
Display dspOper = Display(40,34,BLACK,GRAY1,BLACK);
Display dspArg = Display(70,40,BLACK,GREEN,GRAY1);

unsigned long lastMillis;
unsigned int numPos = 0;
unsigned int CHARSIZE = 12;
long argument1 = 0;
long argument2 = 0;
long result = 0;
char operation = 0;

void setup() {
  Serial.begin(9600);
  canvas.init();
  Tft.layoutMode = TFT_LANDSCAPE;//TFT_PORTRAIT;
  
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
  canvas.add(&btnBack,160,40);

  btnClear.setText("C");
  btnClear.setEventHandler(&clearEventHandler);
  canvas.add(&btnClear,200,40);
  
  btnEqual.setText("=");
  btnEqual.setEventHandler(&equalEventHandler);
  canvas.add(&btnEqual,160,200);
  
  btnPlus.setText("+");
  btnPlus.setEventHandler(&operEventHandler);
  canvas.add(&btnPlus,160,80);

  btnMinus.setText("-");
  btnMinus.setEventHandler(&operEventHandler);
  canvas.add(&btnMinus,160,120);

  btnMult.setText("*");
  btnMult.setEventHandler(&operEventHandler);
  canvas.add(&btnMult,160,160);

  btnDiv.setText("/");
  btnDiv.setEventHandler(&operEventHandler);
  canvas.add(&btnDiv,200,160);

  dspNum.clear();
  dspOper.clear();
  dspArg.clear();
  canvas.add(&dspNum,10,0);
  canvas.add(&dspOper,130,3); 
  canvas.add(&dspArg,170,0); 
  
  Tft.drawRectangle(10,0,230,240,WHITE);
  lastMillis = millis();
}

void loop() {
  if(millis()>lastMillis + 100){ //Debouncing of the touchscreen resistance
    Point tP = getTouchedPoint();
    canvas.touchWidgets(&tP); //Update all buttons on Canvas :)
    lastMillis=millis();
  }
}

void operEventHandler(Button* btn){
  operation = btn->getText()[0];
  switch(operation){
      case 43: // +
        dspOper.setText("+");
        break;
      case 45: // -
        dspOper.setText("-");
        break;
      case 42: // *
        dspOper.setText("*");
        break;
      case 47: // /
        dspOper.setText("/");
        break;
      default:
        return;
  }
  argument1 = dspNum.getNum();
  //dspNum.append(btn->getText());
  dspNum.show();
  dspOper.show();
  //Serial.print("Argument1: "); Serial.println(argument1);
}

void equalEventHandler(Button* btn){
  argument2 = dspArg.getNum();
  switch(operation){
    case 43: // +
      result = argument1 + argument2;
      break;
    case 45: // -
      result = argument1 - argument2;
      break;
    case 42: // *
      result = argument1 * argument2;
      break;
    case 47: // /
      result = argument1 / argument2;
      break;
    default:
      argument2 = 0;
      dspOper.clear();
      dspOper.show();
      dspArg.clear();
      dspArg.show();
      operation = 0;
      return; //Exit without changing anything
  }
  //Serial.print("Argument 1: ");Serial.println(argument1);
  //Serial.print("Argument 2: ");Serial.println(argument2);
  //Serial.print("Result: ");Serial.println(result);
  dspNum.setNum((int)result);
  dspNum.show();
  dspOper.clear();
  dspOper.show();
  dspArg.clear();
  dspArg.show();

  argument1 = 0;
  argument2 = 0;
  operation = 0;
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
  if(operation != 0){
    dspArg.append(btn->getText());
    dspArg.show();
  }else{
    dspNum.append(btn->getText());
    dspNum.show();    
  }
}

void backEventHandler(Button* btn){
  if(operation != 0){
    dspArg.deleteChar();
  }else{
    dspNum.deleteChar();
  }
}

void clearEventHandler(Button* btn){
  dspNum.clear();
  dspNum.show();
  dspOper.clear();
  dspOper.show();
  dspArg.clear();
  dspArg.show();
  argument1 = 0;
  argument2 = 0;
  operation = 0;
  result = 0;
}

Point getTouchedPoint(){
  Point p = ts.getPoint();
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);
  if(Tft.layoutMode == TFT_LANDSCAPE) p.toLandscape();
  return p;
}

