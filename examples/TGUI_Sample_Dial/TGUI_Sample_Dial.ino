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
#include <Dial.h>
#include <Button.h>

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
Dial dial = Dial();
Button btnPlus = Button(20,GRAY1,WHITE,ILI9341_LIGHTGREY);  //Initialization version for round buttons
Button btnMinus = Button(20,GRAY1,WHITE,ILI9341_LIGHTGREY); //Initialization version for round buttons

const int increment = 1;

//==================================
// EVENT HANDLING ROUTINES
//==================================
void btnPlusEventHandler(Button* btn){
  if(dial.getCV() >= dial.scaleMax){
    Serial.println("Maximum value reached");
    return;
  }
  dial.setCV(dial.getCV()+increment);
}

void btnMinusEventHandler(Button* btn){
  if(dial.getCV() <= dial.scaleMin){
    Serial.println("Minimum value reached");
    return;
  }
  dial.setCV(dial.getCV()-increment);
}

//==================================
// SETUP
//==================================
void guiSetup(){
	// Add GUI initialization code here:
  btnPlus.setText("+");
  btnPlus.setEventHandler(&btnPlusEventHandler);
  btnPlus.setDebounce(25);  

  btnMinus.setText("-");
  btnMinus.setEventHandler(&btnMinusEventHandler);
  btnMinus.setDebounce(25);  

  dial.init();
  dial.setSize(50);
  dial.borderWidth = 5;
  dial.setColors(BLACK,GREEN,ILI9341_LIGHTGREY);
  dial.setLimits(60,70,90);
  dial.setHiLimit(75,RED);
  dial.setSP(70,BLUE);
  dial.setLowLimit(70,BLUE);
  dial.setCV(72,false);
      
	canvas.add(&dial,100,160);
	canvas.add(&btnPlus,160,160 - dial.radius);
	canvas.add(&btnMinus,160,160 + dial.radius - btnMinus.h);
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
