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
OptionButton opt1 = OptionButton(40,BLACK,YELLOW,WHITE);
OptionButton opt2 = OptionButton(40,BLACK,GREEN,WHITE);
OptionButton opt3 = OptionButton(40,BLACK,RED,WHITE);

//==================================
// EVENT HANDLING ROUTINES
//==================================
void buttonEventHandler(Button* btn){
  if(btn == &opt1 && btn->touched){
    opt2.touched = false;
    opt2.update();
    opt3.touched = false;
    opt3.update();
  }
  if(btn == &opt2 && btn->touched){
    opt1.touched = false;
    opt1.update();
    opt3.touched = false;
    opt3.update();
  }
  if(btn == &opt3 && btn->touched){
    opt1.touched = false;
    opt1.update();
    opt2.touched = false;
    opt2.update();
  }
  terminal.print(btn->label,btn->fgColor);
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
  opt1.setLabel("Option 1");
  opt1.setEventHandler(&buttonEventHandler);
  opt2.setLabel("Option 2");
  opt2.setEventHandler(&buttonEventHandler);
  opt3.setLabel("Option 3");
  opt3.setEventHandler(&buttonEventHandler);
	canvas.add(&terminal,0,0);
  canvas.add(&opt1,30,80);
  canvas.add(&opt2,30,130);
  canvas.add(&opt3,30,180);
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
