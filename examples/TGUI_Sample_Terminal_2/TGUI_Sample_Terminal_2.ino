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
#include <IconButton.h>
#include <icons.h>
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
IconButton btnSlider = IconButton(60,30,slider_off,slider_on);
Terminal terminal = Terminal(240,50,TERMINAL_SCROLL_UP);

//==================================
// EVENT HANDLING ROUTINES
//==================================
void buttonEventHandler(Button* btn){
	Serial.println((btn->touched)?"Slider On":"Slider Off");
	terminal.print((btn->touched)?"Slider On":"Slider Off",(btn->touched)?ILI9341_GREEN:ILI9341_LIGHTGREY);
	btn->update();
}

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
	btnSlider.setEventHandler(&buttonEventHandler);
	btnSlider.transparentColor = BLACK;
	terminal.setColors(BLACK,WHITE,WHITE);

	canvas.add(&terminal,0,0);
	canvas.add(&btnSlider,90,145);
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
