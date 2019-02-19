/* 
 * Sample template
 * 
 * Created by: Jose Rullan
 * Date: February 16, 2019
 */

// Touch driver specific Canvas include:
#include <Canvas_XPT2046.h>

// Widgets includes here:
#include <Screen.h>
#include <Button.h>
#include <Display.h>

// Architecture specific pins:
// For Wemos Mini D1 (ESP8266)
#define TFT_CS 16 // Wemos D1 Mini D0
#define TFT_DS 15 // Wemos D1 Mini D8
#define TS_CS 0   // Wemos D1 Mini D3
// For MH-ET Live esp32 MiniKit (ESP32)
#define TFT_CS 26 // esp32 MiniKit D0
#define TFT_DS 5  // esp32 MiniKit D8
#define TS_CS 17  // esp32 MiniKit D3

// TGUI's objects declarations:
Canvas_XPT2046 canvas = Canvas_XPT2046(TFT_PORTRAIT,BLACK,TFT_CS,TFT_DS,TS_CS);
Screen screen_1 = Screen(&canvas,0,0,240,260);
Screen screen_2 = Screen(&canvas,0,0,240,260);
Button btnShowDisplay1 = Button();
Button btnShowDisplay2 = Button();
Display display_1 = Display();
Display display_2 = Display();

//==================================
// EVENT HANDLING ROUTINES
//==================================
void showDisplay1EventHandler(Button* btn){
  canvas.setScreen(&screen_1);
}

void showDisplay2EventHandler(Button* btn){
  canvas.setScreen(&screen_2);
}

//==================================
// SETUP
//==================================
void guiSetup(){
  canvas.init();
	
	// Add GUI initialization code here:
  screen_1.bgColor = 0xDFE;  
  display_1.setSize(240,40);
  display_1.setColors(ILI9341_BLUE,WHITE,WHITE);
  display_1.borderWidth = 2;
  display_1.setText("Screen 1",false);
  screen_1.add(&display_1,0,0);

  screen_2.bgColor = 0xfff68f;
  display_2.setSize(240,40);
  display_2.setColors(0xb2ac64,WHITE,WHITE);
  display_2.borderWidth = 2;
  display_2.setText("Screen 2",false);
  screen_2.add(&display_2,0,0);

  btnShowDisplay1.setSize(120,40);
  btnShowDisplay1.setColors(ILI9341_DARKGREY,ILI9341_WHITE,ILI9341_LIGHTGREY);
  btnShowDisplay1.cornerRadius = 8;
  btnShowDisplay1.setText("Screen 1");
  btnShowDisplay1.setEventHandler(&showDisplay1EventHandler);
  btnShowDisplay1.setDebounce(400);
  
  btnShowDisplay2.setSize(120,40);
  btnShowDisplay2.setColors(ILI9341_DARKGREY,ILI9341_WHITE,ILI9341_LIGHTGREY);
  btnShowDisplay2.cornerRadius = 8;
  btnShowDisplay2.setText("Screen 2");
  btnShowDisplay2.setEventHandler(&showDisplay2EventHandler);
  btnShowDisplay2.setDebounce(400);

	canvas.add(&btnShowDisplay1,0,280);
  canvas.add(&btnShowDisplay2,120,280);
  canvas.setScreen(&screen_1);
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
