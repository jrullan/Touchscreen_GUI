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

// Create the objects
//==========================================
Canvas canvas = Canvas(); // Memory used: (storage/ram: 1,676/36)  3,372/228
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



void setup() {
  Serial.begin(9600);
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH);
  canvas.init(TFT_LANDSCAPE);

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
    btnPlus.setDebounce(100);  

    btnMinus.setText("-");
    btnMinus.setEventHandler(&btnMinusEventHandler);
    btnMinus.init();
    btnMinus.setDebounce(100);  

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
    numkey.setNumkeyEventHandler(&numkeyEventHandler);
    numkey.setDebounce(100);
    
  // Add widgets to canvas
  // (Use layout template for coordinates)
  //=========================================
  canvas.add(&button,105,0);
  canvas.add(&dial,55,110);
  canvas.add(&disp,5,0);
  canvas.add(&gauge,110,60);
  canvas.add(&btnPlus,200,60);
  canvas.add(&btnMinus,200,120);
  
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
  canvas.add(&numkey,195,0);
}

void btnPlusEventHandler(Button* btn){
	gauge.setCV(gauge.getCV()+5);
	dial.setCV(gauge.getCV());
	disp.setNum(gauge.getCV());
}

void btnMinusEventHandler(Button* btn){
	if(gauge.getCV() < 5) return;
	gauge.setCV(gauge.getCV()-5);
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
