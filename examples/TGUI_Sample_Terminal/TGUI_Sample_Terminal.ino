/* 
 * Sample template
 * 
 * Created by: Jose Rullan
 * Date: February 16, 2019
 */

// New touch system includes:
#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <TouchCalibration.h>

// Widgets includes here:
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
Terminal terminal = Terminal(240,50,TERMINAL_SCROLL_UP);

//==================================
// EVENT HANDLING ROUTINES
//==================================
void welcomeMessage(){
  terminal.print("Hello World!",YELLOW);
  terminal.print("This is the terminal",GREEN);
  terminal.print("Enjoy!",RED);  
}

//==================================
// SETUP
//==================================
void guiSetup(){
  // Add GUI initialization code here:
  terminal.setColors(BLACK,WHITE,WHITE);

  canvas.add(&terminal,0,0);
  welcomeMessage();
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
