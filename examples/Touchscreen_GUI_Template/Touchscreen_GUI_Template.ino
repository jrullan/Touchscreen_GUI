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

// Global variables
// If you need global variables in your program put them here,
// before the setup() routine.



void setup() {
  Serial.begin(9600);
  
  canvas.init(TFT_LANDSCAPE);

  //Configure the widgets
  //=========================================  
    button.setSize(80,40);
    button.setColors(GRAY1,BLACK,WHITE);
    button.setText("On");
    button.setEventHandler(&buttonEventHandler);
    button.init();
    button.setDebounce(200);  

    dial.setSize(50);
    dial.setColors(GRAY2,YELLOW,GRAY1);
    dial.setLimits(0,50,100);
    dial.init();
    dial.setHiLimit(75,GREEN);
    dial.setLowLimit(25,RED);
    
    disp.setSize(80,40);
    disp.setColors(BLACK,GREEN,GRAY2);
    disp.setText("Hello");
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
    
  // Add widgets to canvas
  // (Use layout template for coordinates)
  //=========================================
  canvas.add(&button,105,0);
  canvas.add(&dial,55,110);
  canvas.add(&disp,5,0);
  canvas.add(&gauge,110,60);
  
  //To use the numkey as a pop-up window
  //do not add the numkey to the canvas yet.
  //Do it when some button is pressed. See
  //buttonEventHandler below for an example.
  //canvas.add(&numkey,198,50);
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

void numkeyEventHandler(Numkey* nk){
  disp.setNum(nk->getNum());
  dial.setHiLimit(nk->getNum(),GREEN);
  gauge.setHiLimit(nk->getNum(),GREEN);
  nk->clear();
  canvas.pop();
}
