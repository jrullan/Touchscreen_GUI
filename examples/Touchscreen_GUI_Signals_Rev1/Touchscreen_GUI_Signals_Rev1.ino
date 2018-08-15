/**************************************************************
 *
 *  Touchscreen_GUI_Template
 *
 *  Use this template as a starting point.
 *  The required libraries "includes" and configuration
 *  sample code is provided to help you quickly create
 *  a working sketch.
 *
 *  This library was developed for Seeedstudio
 *  TouchShield v2.0, based on their TFTv2 and SeeedTouchscreen
 *  libraries.
 *
 *  Comment out lines that are not required for your project.
 *
 *  Author: Jose Rullan
 *  
 *************************************************************/

// Required includes 
#include <SPI.h>

// TouchscreenGUI includes
#include <Button.h>
#include <Display.h>
#include <Canvas_SEEEDTOUCH.h>
//#include <Canvas_FT6206.h>
//#include <Canvas_STMPE610.h>

// Create the objects
//==========================================
Canvas_SEEEDTOUCH canvas = Canvas_SEEEDTOUCH(TFT_LANDSCAPE,BLACK); 
//Canvas_FT6206 canvas = Canvas_FT6206(TFT_LANDSCAPE,BLACK); 
//Canvas_STMPE610 canvas = Canvas_STMPE610(TFT_LANDSCAPE,BLACK);
Button btnLeft = Button(80,80,GRAY1,BLACK,GRAY2); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button btnRight = Button(80,80,GRAY1,BLACK,GRAY2); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button btnHazard = Button(80,80,GRAY1,BLACK,GRAY2); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button btnPlus = Button(40,40,BLUE,WHITE,GRAY2);
Button btnMinus = Button(40,40,BLUE,WHITE,GRAY2);
Display disp = Display();

Button btnLedRight = Button(20,BLACK,BLACK,GRAY2); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button btnLedLeft = Button(20,BLACK,BLACK,GRAY2); // Memory used: (storage/ram: 3,624/63)  6,996/291

// Available Analog Pins: A4,A5
// Available Digital Pins: 0,1,2,3,8,9,10
#define LPIN 12
#define RPIN 13

#define MAX_RATE 500
#define MIN_RATE 100
#define RATE_INC 10

#define MAX_FREQ 50
#define MIN_FREQ 1
#define F_INC 1

#define LEDOFF GRAY2
#define LEDON ILI9341_ORANGE

// Global variables
bool leftActive = false;
bool rightActive = false;
bool hazardActive = false;
int freq = 2;
int blinkSpeed = 1000/freq;
unsigned long lastTime = 0;


void setup() {
  Serial.begin(9600);
  pinMode(LPIN,OUTPUT);
  pinMode(RPIN,OUTPUT);
  
  canvas.init();

  //Configure the widgets
  //=========================================  
  btnLedLeft.setColors(GRAY2,GRAY2,GRAY2);
  btnLedLeft.setText("");
  btnLedLeft.init();

  btnLedRight.setColors(GRAY2,GRAY2,GRAY2);
  btnLedRight.setText("");
  btnLedRight.init();

  btnLeft.setText("<-");
  btnLeft.setEventHandler(&btnLeftEventHandler);
  btnLeft.init();
  
  btnRight.setText("->");
  btnRight.setEventHandler(&btnRightEventHandler);
  btnRight.init();
  
  btnHazard.setText("<->");
  btnHazard.setEventHandler(&btnHazardEventHandler);
  btnHazard.init();

  btnPlus.setText("+");
  btnPlus.setEventHandler(&btnPlusEventHandler);
  btnPlus.init();

  btnMinus.setText("-");
  btnMinus.setEventHandler(&btnMinusEventHandler);
  btnMinus.init();

  disp.setSize(80,40);
  disp.setColors(BLACK,BLUE,GRAY2);
  disp.setText("");
  disp.init();    

  // Add widgets to canvas
  // (Use layout template for coordinates)
  //=========================================
  //canvas.add(&btnLed,140,0);
  canvas.add(&btnLeft,10,70);
  canvas.add(&btnRight,230,70);
  canvas.add(&btnHazard,120,150);
  canvas.add(&btnMinus,80,10);
  canvas.add(&disp,120,10);
  canvas.add(&btnPlus,200,10);
  canvas.add(&btnLedLeft,0,0);
  canvas.add(&btnLedRight,280,0);

  disp.setNum(freq);
  lastTime = millis();
}


void loop() {
  canvas.scan();  
  
  if(btnLeft.touched == true){
    blinkSignal(&leftActive,LPIN);
  }
  
  if(btnRight.touched == true){
    blinkSignal(&rightActive,RPIN);
  }
  
  if(btnHazard.touched == true){
    blinkHazard(&hazardActive,LPIN,RPIN);
  }
}


// Event Handler Functions for buttons
// (one per button)
//==========================================
void btnLeftEventHandler(Button* btn){
  leftActive = true;
  if(btn->touched){
    setBackground(btn,GREEN);
  }else{
    setBackground(btn,GRAY1);
    setBackground(&btnLedLeft,LEDOFF);
    digitalWrite(LPIN,LOW);
  }
  
  //Deactivate other buttons
  btnRight.touched = false;
  setBackground(&btnRight,GRAY1);  
  btnHazard.touched = false;
  setBackground(&btnHazard,GRAY1);
  setBackground(&btnLedRight,LEDOFF);
  
  //Turn off the other LED
  digitalWrite(RPIN,LOW);
}

void btnRightEventHandler(Button* btn){
  rightActive = true;
  if(btn->touched){
    setBackground(btn,GREEN);
  }else{
    setBackground(btn,GRAY1);
    setBackground(&btnLedRight,LEDOFF);
    digitalWrite(RPIN,LOW);
  }
  
  //Deactivate other buttons
  btnLeft.touched = false;
  setBackground(&btnLeft,GRAY1);
  btnHazard.touched = false;
  setBackground(&btnHazard,GRAY1);  
  setBackground(&btnLedLeft,LEDOFF);
  
  //Turn off the other LED
  digitalWrite(LPIN,LOW);
}

void btnHazardEventHandler(Button* btn){
  if(btn->touched){
    setBackground(btn,GREEN);
  }else{
    setBackground(btn,GRAY1);
    
    setBackground(&btnLedLeft,LEDOFF);
    setBackground(&btnLedRight,LEDOFF);
    
    digitalWrite(LPIN,LOW);
    digitalWrite(RPIN,LOW);
  }
  
  //Deactivate other buttons
  btnLeft.touched = false;
  btnRight.touched = false;
  setBackground(&btnLeft,GRAY1);
  setBackground(&btnRight,GRAY1);
}

void btnPlusEventHandler(Button* btn){
  //blinkSpeed = (blinkSpeed >= MIN_RATE + RATE_INC) ? (blinkSpeed-RATE_INC) : MIN_RATE;
  freq = (freq <= MAX_FREQ - F_INC) ? (freq + F_INC) : (MAX_FREQ);
  blinkSpeed = 1000/freq;
  disp.setNum(freq);
}

void btnMinusEventHandler(Button* btn){
  //blinkSpeed = (blinkSpeed <= MAX_RATE - RATE_INC) ? (blinkSpeed+RATE_INC) : MAX_RATE;
  freq = (freq >= MIN_FREQ + F_INC) ? (freq - F_INC) : (MIN_FREQ);
  blinkSpeed = 1000/freq;
  disp.setNum(freq);
}

// Helper routines
//==========================================
void blinkSignal(bool* signal, int pin){
  if(millis() > lastTime + blinkSpeed){
    if(*signal){
      digitalWrite(pin,HIGH);      
      if(pin==LPIN) btnLedLeft.bgColor = LEDON;
      if(pin==RPIN) btnLedRight.bgColor = LEDON;
    }else{
      digitalWrite(pin,LOW);
      if(pin==LPIN) btnLedLeft.bgColor = LEDOFF;
      if(pin==RPIN) btnLedRight.bgColor = LEDOFF;
    }
    btnLedLeft.show();
    btnLedRight.show();
    *signal = !*signal;
    lastTime = millis();
  }
}

void blinkHazard(bool* signal, int pin1, int pin2){
  if(millis() > lastTime + blinkSpeed){
    if(*signal){
      digitalWrite(pin1,HIGH);
      digitalWrite(pin2,HIGH);
      btnLedLeft.bgColor = LEDON;
      btnLedRight.bgColor = LEDON;
    }else{
      digitalWrite(pin1,LOW);
      digitalWrite(pin2,LOW);
      btnLedLeft.bgColor = LEDOFF;
      btnLedRight.bgColor = LEDOFF;
    }
    btnLedLeft.show();
    btnLedRight.show();
    *signal = !*signal;
    lastTime = millis();
  }
}


void setBackground(Button* btn, int color){
  btn->setColors(color,btn->fgColor,btn->borderColor);
  btn->show();
}