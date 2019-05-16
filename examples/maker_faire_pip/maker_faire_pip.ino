/* 
 * Sample template
 * 
 * Created by: Jose Rullan
 * Date: February 16, 2019
 */

// Touch driver specific Canvas include:
#include <Canvas_XPT2046.h>
#include <neotimer.h>

// Widgets includes here:
#include <Terminal.h>
#include <Trend.h>

#include <IconButton.h>
#include "person_off.c"
#include "person_on.c"

// For MH-ET Live esp32 MiniKit (ESP32)
#define TFT_CS 26 // esp32 MiniKit D0
#define TFT_DS 5  // esp32 MiniKit D8
#define TS_CS 17  // esp32 MiniKit D3

// TGUI's objects declarations:
Canvas_XPT2046 canvas = Canvas_XPT2046(TFT_LANDSCAPE,BLACK,TFT_CS,TFT_DS,TS_CS);
Terminal terminal = Terminal(180,50,TERMINAL_SCROLL_UP);
Trend trend = Trend(220, 100, 0, 50, 100);
Neotimer terminalUpdateTimer = Neotimer(250);
Neotimer trendUpdateTimer = Neotimer(10);
IconButton btnPerson = IconButton(80,100,person_off,person_on);

int val = 0;
uint8_t yVal = 0;
uint8_t dir = 1;
float pi = 3.14159265359;
uint8_t degreeIncrement = 15;

//==================================
// EVENT HANDLING ROUTINES
//==================================
void welcomeMessage(){
  terminal.print("Hello World!",YELLOW);
  terminal.print("This is the terminal",GREEN);
  terminal.print("Enjoy!",RED);  
}

void personButtonEventHandler(Button* btn){
  Serial.println((btn->touched)?"Person On":"Person Off");
  btn->update();
}


//==================================
// SETUP
//==================================
void guiSetup(){
  // Add GUI initialization code here:
  terminal.setColors(BLACK,GREEN,GREEN);

  trend.fgColor = GREEN;
  trend.borderColor = GREEN;
  trend.setSP(50,WHITE);
  trend.setWindow(0,31);
  trend.clear();

  btnPerson.setEventHandler(&personButtonEventHandler);
  //btnPerson.transparentColor = BLACK;

  canvas.add(&trend,0,0);  
  canvas.add(&btnPerson,240,0);
  canvas.add(&terminal,40,120);

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

  if(btnPerson.touched){
    if(trendUpdateTimer.repeat()){
      val = val + degreeIncrement;
      if(val == 360) val = 0;
      yVal = 50 * sin(val * pi / 180.0) + 50;    
      trend.addValue(yVal);  
    }
    
    if(terminalUpdateTimer.repeat()){
      String str = "x = " + String(val) + ", y = " + String(yVal);
      terminal.print(str);
    }    
  }

  
}
