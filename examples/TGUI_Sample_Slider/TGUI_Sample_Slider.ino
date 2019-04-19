// Touch driver specific Canvas include:
#include <Canvas_XPT2046.h>

// Widgets includes here:
#include <Slider.h>
#include <Display.h>


// Architecture specific pins:
// For MH-ET Live esp32 MiniKit (ESP32)
#define TFT_CS 26 // esp32 MiniKit D0
#define TFT_DS 5  // esp32 MiniKit D8
#define TS_CS 17  // esp32 MiniKit D3

// TGUI's objects declarations:
Canvas_XPT2046 canvas = Canvas_XPT2046(TFT_PORTRAIT,BLACK,TFT_CS,TFT_DS,TS_CS);
Slider blueSlider = Slider();
Display blueDisp = Display(8);
Slider greenSlider = Slider();
Display greenDisp = Display(8);
Slider redSlider = Slider();
Display redDisp = Display(8);

//==================================
// EVENT HANDLING ROUTINES
//==================================
void sliderEventHandler(Slider* sld){
  Serial.print("Slider Pressed, CV = ");
  Serial.println(sld->currentValue);
  Serial.println("Calling Display's setNum");
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
}

//==================================
// SETUP
//==================================
void guiSetup(){  
  blueSlider.setDebounce(0);
  blueSlider.setSize(40,240);
  blueSlider.setColors(BLACK,BLUE,WHITE);
  blueSlider.setEventHandler(&sliderEventHandler);
  
  blueDisp.setSize(40,40);
  blueDisp.setColors(ILI9341_BLUE,ILI9341_WHITE,ILI9341_LIGHTGREY);
  blueDisp.setText("",false); 
  
  greenSlider.setDebounce(0);
  greenSlider.setSize(40,240);
  greenSlider.setColors(BLACK,GREEN,WHITE);
  greenSlider.setEventHandler(&sliderEventHandler);

  greenDisp.setSize(40,40);
  greenDisp.setColors(ILI9341_DARKGREEN,ILI9341_WHITE,ILI9341_LIGHTGREY);
  greenDisp.setText("",false);

  redSlider.setDebounce(0);
  redSlider.setSize(40,240);
  redSlider.setColors(BLACK,RED,WHITE);
  redSlider.setEventHandler(&sliderEventHandler);

  redDisp.setSize(40,40);
  redDisp.setColors(ILI9341_RED,ILI9341_WHITE,ILI9341_LIGHTGREY);
  redDisp.setText("",false);

  canvas.add(&blueDisp, 20, 10);
  canvas.add(&blueSlider, 20, 60);
  
  canvas.add(&greenDisp, 100, 10);
  canvas.add(&greenSlider, 100, 60);  

  canvas.add(&redDisp, 180, 10);
  canvas.add(&redSlider, 180, 60); 
}

void setup() {
  Serial.begin(115200);
  guiSetup();
}

void loop() {
  canvas.scan();
}
