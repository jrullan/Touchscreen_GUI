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


// Create the objects
//==========================================
Canvas canvas = Canvas(); // Memory used: (storage/ram: 1,676/36)  3,372/228
Button btnLed = Button(20,BLACK,BLACK,GRAY2); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button btnLeft = Button(80,40,GRAY1,BLACK,GRAY2); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button btnRight = Button(80,40,GRAY1,BLACK,GRAY2); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button btnHazard = Button(80,40,GRAY1,BLACK,GRAY2); // Memory used: (storage/ram: 3,624/63)  6,996/291

// Global variables
const int leftPin = 22;
const int rightPin = 24;
bool leftActive = false;
bool rightActive = false;
bool hazardActive = false;
int blinkSpeed = 400;
unsigned long lastTime = 0;


void setup() {
  Serial.begin(9600);
  pinMode(leftPin,OUTPUT);
  pinMode(rightPin,OUTPUT);
  
  canvas.init(TFT_LANDSCAPE);

  //Configure the widgets
  //=========================================  
    //button.setSize(80,40);
    //button.setColors(GRAY1,BLACK,WHITE);
    
    btnLed.init();
    
    btnLeft.setText("<-");
    btnLeft.setEventHandler(&btnLeftEventHandler);
    btnLeft.init();
    
    btnRight.setText("->");
    btnRight.setEventHandler(&btnRightEventHandler);
    btnRight.init();
    
    btnHazard.setText("<->");
    btnHazard.setEventHandler(&btnHazardEventHandler);
    btnHazard.init();


  // Add widgets to canvas
  // (Use layout template for coordinates)
  //=========================================
  //canvas.add(&btnLed,140,0);
  canvas.add(&btnLeft,55,50);
  canvas.add(&btnRight,185,50);
  canvas.add(&btnHazard,120,150);
  
  lastTime = millis();
}


void loop() {
  
  canvas.scan();  
  
  if(btnLeft.touched == true){
    blinkSignal(&leftActive,leftPin);
  }
  
  if(btnRight.touched == true){
    blinkSignal(&rightActive,rightPin);
  }
  
  if(btnHazard.touched == true){
    blinkHazard(&hazardActive,leftPin,rightPin);
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
    digitalWrite(leftPin,LOW);
  }
  
  //Deactivate other buttons
  btnRight.touched = false;
  setBackground(&btnRight,GRAY1);  
  btnHazard.touched = false;
  setBackground(&btnHazard,GRAY1);
  
  //Turn off the other LED
  digitalWrite(rightPin,LOW);
}

void btnRightEventHandler(Button* btn){
  rightActive = true;
  if(btn->touched){
    setBackground(btn,GREEN);
  }else{
    setBackground(btn,GRAY1);
    digitalWrite(rightPin,LOW);
  }
  
  //Deactivate other buttons
  btnLeft.touched = false;
  setBackground(&btnLeft,GRAY1);
  btnHazard.touched = false;
  setBackground(&btnHazard,GRAY1);  
  
  //Turn off the other LED
  digitalWrite(leftPin,LOW);
}

void btnHazardEventHandler(Button* btn){
  if(btn->touched){
    setBackground(btn,GREEN);
  }else{
    setBackground(btn,GRAY1);
    digitalWrite(leftPin,LOW);
    digitalWrite(rightPin,LOW);
  }
  
  //Deactivate other buttons
  btnLeft.touched = false;
  btnRight.touched = false;
  setBackground(&btnLeft,GRAY1);
  setBackground(&btnRight,GRAY1);
}

// Helper routines
//==========================================
void blinkSignal(bool* signal, int pin){
  if(millis() > lastTime + blinkSpeed){
    if(*signal){
      digitalWrite(pin,HIGH);
    }else{
      digitalWrite(pin,LOW);
    }
    *signal = !*signal;
    lastTime = millis();
  }
}

void blinkHazard(bool* signal, int pin1, int pin2){
  if(millis() > lastTime + blinkSpeed){
    if(*signal){
      digitalWrite(pin1,HIGH);
      digitalWrite(pin2,HIGH);
    }else{
      digitalWrite(pin1,LOW);
      digitalWrite(pin2,LOW);
    }
    *signal = !*signal;
    lastTime = millis();
  }
}

void setBackground(Button* btn, int color){
  btn->setColors(color,btn->fgColor,btn->borderColor);
  btn->show();
}
