/* 
 * Sample template
 * 
 * Button Example
 * 
 * Created by: Jose Rullan
 * Date: February 16, 2019
 */

// New touch system includes:
#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <TouchCalibration.h>

// Widgets includes here:
#include <Button.h>
#include <Terminal.h>


// Architecture specific pins:
#ifdef ESP32
  #define TFT_CS 26
  #define TFT_DS 5
  #define TS_CS 17
#else
  #define TFT_CS 16
  #define TFT_DS 15
  #define TS_CS 0
#endif

// TGUI's objects declarations:
TouchDriver_XPT2046 touchDriver(TS_CS);
TouchMapper mapper(&touchDriver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, TFT_CS, TFT_DS, &mapper);
// Legacy API: Canvas_XPT2046 canvas = Canvas_XPT2046(TFT_PORTRAIT,BLACK,TFT_CS,TFT_DS,TS_CS);
Button button = Button();
Terminal terminal = Terminal(240,50,TERMINAL_SCROLL_UP);

int times = 0;

//==================================
// EVENT HANDLING ROUTINES
//==================================
/*
 * This example shows how to use print to show an integer number
 * within the message using the %d placeholder in place of the 
 * number variable.
 */
void buttonEventHandler(Button* btn){
  terminal.print("Button pressed %d times",++times);
}

//==================================
// SETUP
//==================================
void guiSetup(){
  // Add GUI initialization code here:

  terminal.setColors(canvas.bgColor,WHITE,WHITE);

  button.setColors(ILI9341_DARKGREY,ILI9341_WHITE,ILI9341_LIGHTGREY);
  button.setSize(80,40);
  button.cornerRadius = 8;
  button.setText("Press");
  button.setDebounce(200);
  button.setEventHandler(&buttonEventHandler);

  canvas.add(&terminal,0,0);
  canvas.add(&button,80,140);
}

//==================================
// Application Code
//==================================

void setup() {
  Serial.begin(115200);
  guiSetup();
}

void loop() {
  canvas.scan();
}
