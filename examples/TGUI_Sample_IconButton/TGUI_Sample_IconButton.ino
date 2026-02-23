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

//==================================
// EVENT HANDLING ROUTINES
//==================================
void buttonEventHandler(Button* btn){
	Serial.println((btn->touched)?"Slider On":"Slider Off");
	btn->update();
}

//==================================
// SETUP
//==================================
void guiSetup(){
	// Add GUI initialization code here:
	btnSlider.setEventHandler(&buttonEventHandler);
	btnSlider.transparentColor = BLACK;
		
	canvas.add(&btnSlider,90,145);
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
