/**************************************************************
 *
 *  Touchscreen_GUI_Template
 *
 *  Use this template as a starting point.
 *  The required libraries "includes" and configuration
 *  sample code is provided to help you quickly create
 *  a working sketch.
 *
 *  This library was originally developed for Seeedstudio
 *  TouchShield v2.0, based on their TFTv2 and SeeedTouchscreen
 *  libraries. It now includes support for Adafruit's Capacitive
 *  touchscreen TFT as well.
 *  
 *	This library is intended to provide a minimal
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
 *	Seeedstudio - original code
 *  Adafruit - underlying code for ILI9341, GFX, FT6206, etc.
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

// Create the objects
//==========================================
//Canvas canvas = Canvas(TFT_LANDSCAPE,BLACK); // or Canvas(TFT_LANDSCAPE,BLACK,TOUCHTYPE_SEEEDSTUDIO_RESISTIVE)
Canvas canvas = Canvas(TFT_LANDSCAPE,BLACK,TOUCHTYPE_ADAFRUIT_CAPACITIVE); 
Button button = Button();
Dial dial = Dial();       
Display disp = Display(); 
Gauge gauge = Gauge();    
Numkey numkey = Numkey();
Button btnPlus = Button(20,GRAY1,BLACK,WHITE);	//Initialization version for round buttons
Button btnMinus = Button(20,GRAY1,BLACK,WHITE); //Initialization version for round buttons

const char increment = 1;

void setup() {
  Serial.begin(115200);
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
    btnPlus.setDebounce(25);  

    btnMinus.setText("-");
    btnMinus.setEventHandler(&btnMinusEventHandler);
    btnMinus.init();
    btnMinus.setDebounce(25);  

    dial.setSize(50);
    dial.setColors(GRAY2,YELLOW,GRAY1);
    dial.setLimits(0,50,100);
    dial.init();
    dial.setHiLimit(75,GREEN);
    dial.setSP(50,BLUE);
    dial.setLowLimit(25,RED);
    
    disp.setSize(80,40);
    disp.setColors(BLACK,GREEN,GRAY2);
    disp.setText("");
    disp.init();
    
    gauge.setSize(40,100);
    gauge.setColors(BLACK,YELLOW,WHITE);
    gauge.setLimits(0,50,100);
    gauge.init();
    gauge.setHiLimit(75,GREEN);
    gauge.setSP(50,BLUE);
    gauge.setLowLimit(25,RED);
    
    numkey.setSize(120,180);
    numkey.setColors(GRAY1,BLACK,WHITE);
    numkey.init();
    numkey.setNumkeyEventHandler(&numkeyEventHandler);
    numkey.setDebounce(100);
    
  // Add widgets to canvas
  // (Use layout template for coordinates)
  //=========================================
  canvas.add(&disp,55,40);
  canvas.add(&button,155,40);
  canvas.add(&dial,95,150);
  canvas.add(&gauge,160,100);
  canvas.add(&btnPlus,240,100);
  canvas.add(&btnMinus,240,160);
  
  //Numkey Notes:
  //The numkey widget is meant to be a pop-up
  //kind of widget. So it is added to the canvas 
  //when it is required. Upon pressing the = sign
  //it will call it's event handler and after that
  //it will automatically be removed from the canvas.
  //To prevent this behavior then set
  //	numkey.isPopUp = false; (By default = true)
	//See buttonEventHandler below for an example.

}


void loop() {
  canvas.scan();
}


// Event Handler Functions for buttons
//==========================================
void buttonEventHandler(Button* btn){
  // Example of a pop-up num keypad.
  // When button is pressed, the numkey
  // is added to the canvas, and it is rendered
  // automatically.
  canvas.add(&numkey,100,30);
}

void btnPlusEventHandler(Button* btn){
	gauge.setCV(gauge.getCV()+increment);
	dial.setCV(gauge.getCV());
	disp.setNum(gauge.getCV());
}

void btnMinusEventHandler(Button* btn){
	if(gauge.getCV() < increment) return;
	gauge.setCV(gauge.getCV()-increment);
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