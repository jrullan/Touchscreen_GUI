/*
	Sample template
	
	This example shows how to use sliders.
	
	There are three sliders in this application. 
	One for Blue, one for Green and one for Red.
	
	A display at the top of each slider shows the current value
	of the slider. 
	
	A main display changes its background color based on the 
	combined value of each slider.

	Created by: Jose Rullan
	updated: January 11, 2020
 */


// Touch driver specific Canvas include:
// For Lolin TFT-2.4
#include <Canvas_XPT2046.h>

// Widgets includes here:
#include <Slider.h>
#include <Display.h>

// Architecture specific pins:

// For Wemos Mini D1 (ESP8266)
//#define TFT_CS 16 // Wemos D1 Mini D0
//#define TFT_DS 15 // Wemos D1 Mini D8
//#define TS_CS 0   // Wemos D1 Mini D3

// For MH-ET Live esp32 MiniKit (ESP32)
#define TFT_CS 26 // esp32 MiniKit D0
#define TFT_DS 5  // esp32 MiniKit D8
#define TS_CS 17  // esp32 MiniKit D3

// TGUI's objects declarations:
Canvas_XPT2046 canvas = Canvas_XPT2046(TFT_PORTRAIT,BLACK,TFT_CS,TFT_DS,TS_CS);
Slider blueSlider = Slider();
Display blueDisp = Display();
Slider greenSlider = Slider();
Display greenDisp = Display();
Slider redSlider = Slider();
Display redDisp = Display();
Display demoDisp = Display();

//==================================
// EVENT HANDLING ROUTINES
//==================================
/*
	This routine handles all of the sliders events.
	Inside the routine, it checks which slider generated the event
	and updates its corresponding value display and the color of the
	demo display.
*/
void sliderEventHandler(Slider* sld){

  char buf[8];
  String value = String(sld->currentValue);
  value.toCharArray(buf,8);

  if(sld == &blueSlider){
    blueDisp.setText(buf,false);
    blueDisp.update();
  }

  if(sld == &greenSlider){
    greenDisp.setText(buf,false);
    greenDisp.update();
  }

  if(sld == &redSlider){
    redDisp.setText(buf,false);
    redDisp.update();
  }  

  int red = map(redSlider.currentValue,0,100,0,255);
  int green = map(greenSlider.currentValue,0,100,0,255);
  int blue = map(blueSlider.currentValue,0,100,0,255);
  int newColor = canvas.tft->color565(red,green,blue);

  demoDisp.bgColor = newColor;
  demoDisp.update();
}

//==================================
// SETUP
//==================================
void guiSetup(){ 
  // Display to show the resulting color
  demoDisp.setSize(80,40);
  demoDisp.setColors(ILI9341_BLACK,ILI9341_WHITE,ILI9341_LIGHTGREY);
  demoDisp.setText("",false); 
   
  // Blue slider elements
  blueSlider.setDebounce(0);
  blueSlider.setSize(40,140);
  blueSlider.setColors(BLACK,BLUE,WHITE);
  blueSlider.setEventHandler(&sliderEventHandler);
  blueDisp.setSize(40,40);
  blueDisp.setColors(ILI9341_BLUE,ILI9341_WHITE,ILI9341_LIGHTGREY);
  blueDisp.setText("",false); 
  
  // Green slider elements
  greenSlider.setDebounce(0);
  greenSlider.setSize(40,140);
  greenSlider.setColors(BLACK,GREEN,WHITE);
  greenSlider.setEventHandler(&sliderEventHandler);
  greenDisp.setSize(40,40);
  greenDisp.setColors(ILI9341_DARKGREEN,ILI9341_WHITE,ILI9341_LIGHTGREY);
  greenDisp.setText("",false);

  // Red slider elements
  redSlider.setDebounce(0);
  redSlider.setSize(40,140);
  redSlider.setColors(BLACK,RED,WHITE);
  redSlider.setEventHandler(&sliderEventHandler);
  redDisp.setSize(40,40);
  redDisp.setColors(ILI9341_RED,ILI9341_WHITE,ILI9341_LIGHTGREY);
  redDisp.setText("",false);

  canvas.add(&demoDisp, 80, 25);
  canvas.add(&redDisp, 180, 110);
  canvas.add(&redSlider, 180, 160);   
	canvas.add(&greenDisp, 100, 110);
  canvas.add(&greenSlider, 100, 160);  
  canvas.add(&blueDisp, 20, 110);
  canvas.add(&blueSlider, 20, 160);  
}

void setup() {
  Serial.begin(115200);
  guiSetup();
}

void loop() {
  canvas.scan();
}
