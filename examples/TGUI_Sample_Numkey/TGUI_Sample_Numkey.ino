/* 
 * Sample template
 * 
 * Created by: Jose Rullan
 * Date: February 16, 2019
 */

// Touch driver specific Canvas include:
#include <Canvas_XPT2046.h>

// Widgets includes here:
#include <Numkey.h>

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
Numkey numkey = Numkey();

//==================================
// EVENT HANDLING ROUTINES
//==================================
void numkeyEventHandler(Numkey* nk){
	Serial.println(nk->getText());
	nk->clear();
}

//==================================
// SETUP
//==================================
void guiSetup(){
	// Add GUI initialization code here:
	numkey.setSize(120,180);
	numkey.setColors(ILI9341_DARKGREY,ILI9341_BLACK,ILI9341_WHITE);
	numkey.setEventHandler(&numkeyEventHandler);
	numkey.setDebounce(100);
	numkey.autoremove = false;

	canvas.add(&numkey,60,50);
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
