/* 
 * Sample template
 * 
 * Button Example
 * 
 * Created by: Jose Rullan
 * Date: February 16, 2019
 */

// Touch driver specific Canvas include:
#include <Canvas_XPT2046.h>

// Widgets includes here:
#include <Button.h>
#include <Terminal.h>


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
Button button = Button();
Terminal terminal = Terminal(240,50,TERMINAL_SCROLL_UP);

int times = 0;

//==================================
// EVENT HANDLING ROUTINES
//==================================
/*
 * This example shows how to use print to show an integer number
 * within the message using the %d placeholder in place of the 
 * number variable.
 */
void buttonEventHandler(Button* btn){
  terminal.print("Button pressed %d times",++times);
}

//==================================
// SETUP
//==================================
void guiSetup(){
  // Add GUI initialization code here:

  terminal.setColors(canvas.bgColor,WHITE,WHITE);

  button.setColors(ILI9341_DARKGREY,ILI9341_WHITE,ILI9341_LIGHTGREY);
  button.setSize(80,40);
  button.cornerRadius = 8;
  button.setText("Press");
  button.setDebounce(200);
  button.setEventHandler(&buttonEventHandler);

  canvas.add(&terminal,0,0);
  canvas.add(&button,80,140);
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
