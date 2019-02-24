/* 
 * Sample template
 * 
 * Created by: Jose Rullan
 * Date: February 16, 2019
 */

// Touch driver specific Canvas include:
#include <Canvas_XPT2046.h>

// Widgets includes here:
#include <Terminal.h>
#include <OptionButton.h>

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
Terminal terminal = Terminal(240,50,TERMINAL_SCROLL_UP);
OptionButton opt = OptionButton(40,BLACK,YELLOW,WHITE);

//==================================
// EVENT HANDLING ROUTINES
//==================================
void buttonEventHandler(Button* btn){
	Serial.println((btn->touched)? "Option On" : "Option Off");
	terminal.print((btn->touched)? "Option On" : "Option Off",(btn->touched)?ILI9341_GREEN:ILI9341_LIGHTGREY);
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
	canvas.init();

	// Add GUI initialization code here:
	terminal.setColors(BLACK,WHITE,WHITE);
  opt.setLabel("On");
  opt.setEventHandler(&buttonEventHandler);

	canvas.add(&terminal,0,0);
  canvas.add(&opt,100,190);
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
