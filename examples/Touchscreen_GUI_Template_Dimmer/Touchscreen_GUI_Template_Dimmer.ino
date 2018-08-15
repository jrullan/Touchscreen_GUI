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
 *  libraries. This library is intended to provide a minimal
 *  but useful collection of reusable widgets that can be used
 *  to create a Graphical User Interface (GUI) for any Arduino 
 *  project.
 *
 *  Comment out (or delete) lines that are not required for your project.
 *
 *  Author: Jose Rullan
 *	Release Date: 03/28/2015  -- Arduino Day 2015!!!
 *	Release Version: 0.1 beta
 *
 *	License: Whatever only requires to give credit to original 
 *	author (i.e. Jose Rullan)
 *
 *	Credits:
 *	
 *  
 *************************************************************/

// Required includes 
#include <SPI.h>

// TouchscreenGUI includes
#include <Button.h>
#include <Dial.h>
#include <Display.h>
#include <Gauge.h>
#include <Numkey.h>
#include <Canvas_SEEEDTOUCH.h>
//#include <Canvas_FT6206.h>
//#include <Canvas_STMPE610.h>

// Create the objects
//==========================================
Canvas_SEEEDTOUCH canvas = Canvas_SEEEDTOUCH(TFT_LANDSCAPE,BLACK); 
//Canvas_FT6206 canvas = Canvas_FT6206(TFT_LANDSCAPE,BLACK); 
//Canvas_STMPE610 canvas = Canvas_STMPE610(TFT_LANDSCAPE,BLACK);
Button button = Button(); // Memory used: (storage/ram: 3,624/63)  6,996/291
Dial dial = Dial();       // Memory used: (storage/ram: 4,760/64)  11756/355
Display disp = Display(); // Memory used: (storage/ram: 484/37)    12,240/392
Gauge gauge = Gauge();    // Memory used: (storage/ram: 1,470/52)  13710/444
Numkey numkey = Numkey(); // Memory used: (storage/ram: 2,370/59)  16,080/503
Button btnPlus = Button(20,GRAY1,BLACK,WHITE);	//Initialization version for round buttons
Button btnMinus = Button(20,GRAY1,BLACK,WHITE); //Initialization version for round buttons

// Global variables
// If you need global variables in your program put them here,
// before the setup() routine.
const int rightPin = 13;// 45;  //Using Mega PWM pin
const int leftPin =12;// 44;   //Using Mega PWM pin
int lightVal = 0;
bool passLock = true;

void setup() {
  Serial.begin(9600);
  pinMode(leftPin,OUTPUT);
  pinMode(rightPin,OUTPUT);
  
  canvas.init();

  //Configure the widgets
  //=========================================  
	button.setSize(80,40);
	button.setColors(GRAY1,BLACK,WHITE);
	button.setText("Change");
	button.setEventHandler(&buttonEventHandler);
	button.init();
	button.setDebounce(100);  

	btnPlus.setText("+");
	btnPlus.setEventHandler(&btnPlusEventHandler);
	btnPlus.init();
	btnPlus.setDebounce(10);  

	btnMinus.setText("-");
	btnMinus.setEventHandler(&btnMinusEventHandler);
	btnMinus.init();
	btnMinus.setDebounce(10);  

	dial.setSize(50);
	dial.setColors(GRAY2,YELLOW,GRAY1);
	dial.setLimits(0,50,100);
	dial.init();
	dial.setHiLimit(75,GREEN);
	dial.setLowLimit(25,RED);
	
	disp.setSize(80,40);
	disp.setColors(BLACK,GREEN,GRAY2);
	disp.setText("");
	disp.init();
	
	gauge.setSize(40,100);
	gauge.setColors(GRAY2,YELLOW,WHITE);
	gauge.setLimits(0,50,100);
	gauge.init();
	gauge.setHiLimit(75,GREEN);
	gauge.setLowLimit(25,RED);
	
	numkey.setSize(120,180);
	numkey.setColors(GRAY1,BLACK,WHITE);
	numkey.init();
	numkey.setDebounce(200);

	// Password protect the application
	checkPassword();
	
	// If password is Ok, redirect numkey event  handler    
	numkey.setNumkeyEventHandler(&numkeyEventHandler);
    
  // Add widgets to canvas
  // (Use layout template for coordinates)
  //=========================================
  canvas.add(&dial,55,55);
  canvas.add(&disp,120,0);
  canvas.add(&gauge,120,50);
  canvas.add(&button,210,0);
  canvas.add(&btnPlus,230,50);
  canvas.add(&btnMinus,230,110);
  canvas.setDebounce(0);
  //To use the numkey as a pop-up window
  //do not add the numkey to the canvas yet.
  //Do it when some button is pressed. See
  //buttonEventHandler below for an example.
  //canvas.add(&numkey,198,50);
}


void loop() {
  canvas.scan();
  pwmRightPin(dial.getCV()*2.5); //PWM range is 0-255
}


// This function sets the numkey to act as a 
// password lock keypad. The event is set to check for the correct
// password and autoremove is turned Off so the numkey
// stays on the screen after each wrong password entry.
void checkPassword(){
  numkey.setNumkeyEventHandler(&numkeyPassEventHandler);
  numkey.autoremove = false;    
  canvas.add(&numkey,100,0);
  while(passLock){
    canvas.scan();
  }  
}


// Event Handler Functions for buttons
//==========================================
void buttonEventHandler(Button* btn){
  // Example of a pop-up num keypad.
  // When button is pressed, the numkey
  // is added to the canvas, and it is rendered
  // automatically.
  canvas.add(&numkey,195,0);
}

void btnPlusEventHandler(Button* btn){
	gauge.setCV(gauge.getCV()+10);
	dial.setCV(gauge.getCV());
	disp.setNum(gauge.getCV());
}

void btnMinusEventHandler(Button* btn){
	if(gauge.getCV() < 10) return;
	gauge.setCV(gauge.getCV()-10);
	dial.setCV(gauge.getCV());
	disp.setNum(gauge.getCV());
}

void numkeyEventHandler(Numkey* nk){
  if(nk->getTextSize()>0){
    disp.setNum(nk->getNum());
    dial.setCV(nk->getNum());
    gauge.setCV(nk->getNum());
    nk->clear();
  }
}

// Event handler to check for correct password
void numkeyPassEventHandler(Numkey* nk){
  if(nk->getTextSize()>0){
    if(nk->getNum() == 123){ //Correct Password
      passLock = false;
      nk->autoremove = true;    //Set to autoremove
    }
    nk->clear();
  }
}

void pwmRightPin(int value){
  analogWrite(leftPin,value);
  analogWrite(rightPin,value);
}
