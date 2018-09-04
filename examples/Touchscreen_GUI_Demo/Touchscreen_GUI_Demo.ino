/*
 * This demo demonstrates the power and 
 * flexibility of this library.
 * This demo runs on an Arduino Uno or 
 * any other Arduino compatible microcontrollers
 * including:
 * ESP8266
 * ESP32
 * STM32
 * 
 * The demo showcases the following features:
 * Display widgets used to show a text
 * Buttons and Round Buttons
 * Multiple Screens
 * Use of the Numkey widget as a password entry
 * Dial indicator
 * Terminal widget useful for on-screen information
 * 
 * Created by: Jose Rullan
 * Date: September 3, 2018
 */

// Use either of these three and 
// its corresponding class
#include <Canvas_SEEEDTOUCH.h>
//#include <Canvas_FT6206.h>
//#include <Canvas_STMPE610.h>

#include <Screen.h>
#include <Terminal.h>
#include <Display.h>
#include <Button.h>
#include <Dial.h>
#include <Numkey.h>

// Use either of these three and
// its corresponding include above
Canvas_SEEEDTOUCH canvas = Canvas_SEEEDTOUCH(TFT_PORTRAIT,BLACK);
//Canvas_FT6206 canvas = Canvas_FT6206(TFT_PORTRAIT,BLACK);
//Canvas_STMPE610 canvas = Canvas_STMPE610(TFT_PORTRAIT,BLACK);

Display header = Display(240,50,BLUE,WHITE,WHITE,20);
Display title = Display(20);
Terminal terminal = Terminal(240,50);
Numkey numkey = Numkey();

Screen screen_main = Screen(&canvas);
Button btnMain = Button();
Button btnClearTerminal = Button();

Screen screen_dial = Screen(&canvas);
Button btnDial = Button();
Dial dial = Dial();
Button btnPlus = Button(20,GRAY1,WHITE,ILI9341_LIGHTGREY);  //Initialization version for round buttons
Button btnMinus = Button(20,GRAY1,WHITE,ILI9341_LIGHTGREY); //Initialization version for round buttons

const char increment = 1;

//==================================
// EVENT HANDLING ROUTINES
//==================================
void btnMainEventHandler(Button* btn){
  terminal.print("Switched to Main Screen");
  header.setText("Main");
  canvas.setScreen(&screen_main);
}

void btnDialEventHandler(Button* btn){
  // Show the numkey to enter a password
  // The numkey event handler will verify the password
  // and change the screen if the password is correct
  canvas.add(&numkey,60,50);
}

void btnClearTerminalEventHandler(Button* btn){
  terminal.clear();
  Serial.println("Terminal Cleared");
}

void btnPlusEventHandler(Button* btn){
  if(dial.getCV() >= dial.scaleMax){
    terminal.print("Maximum value reached",RED);
    return;
  }
  dial.setCV(dial.getCV()+increment);
}

void btnMinusEventHandler(Button* btn){
  if(dial.getCV() <= dial.scaleMin){
    terminal.print("Minimum value reached",BLUE);
    return;
  }
  dial.setCV(dial.getCV()-increment);
}

void numkeyEventHandler(Numkey* nk){
  char* password = "1234";
  if(nk->getTextSize() == Widget::getTextLength(password)){
    bool match = true;
    for(int i=0;i<nk->getTextSize();i++){
      if(nk->contents.text[i] != password[i]) match = false; 
    }
    if(match){
      header.setText("Dial");
      canvas.setScreen(&screen_dial);
      terminal.print("Password OK",GREEN);
      nk->clear();
      return;
    }
  }
  terminal.print("Wrong password entered",RED);
  nk->clear();
}

//==================================
// SETUP
//==================================
void setup() {
  Serial.begin(115200);
  canvas.init();

  // ===== MAIN SCREEN =====
  screen_main.bgColor = 0xDFE;
  title.setSize(240,40);
  title.setColors(screen_main.bgColor,GRAY2,screen_main.bgColor);
  title.borderWidth = 0;
  title.setText("GUI DEMO");
  screen_main.add(&title,0,120);

  // ===== SCREEN TWO =====
  btnPlus.setText("+");
  btnPlus.setEventHandler(&btnPlusEventHandler);
  btnPlus.init();
  btnPlus.setDebounce(25);  

  btnMinus.setText("-");
  btnMinus.setEventHandler(&btnMinusEventHandler);
  btnMinus.init();
  btnMinus.setDebounce(25);  

  dial.init();
  dial.setSize(50);
  dial.borderWidth = 5;
  dial.setColors(WHITE,GRAY2,GRAY2);
  dial.setLimits(60,70,90);
  dial.setHiLimit(75,RED);
  dial.setSP(70,BLUE);
  dial.setLowLimit(70,BLUE);
  dial.setCV(72);

  screen_dial.bgColor = screen_main.bgColor;//0xf80f;
  screen_dial.add(&dial,100,130);
  screen_dial.add(&btnPlus,160,80);
  screen_dial.add(&btnMinus,160,140);

  // ===== CANVAS GENERAL ITEMS  =====  
  btnMain.setSize(80,40);
  btnMain.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnMain.setEventHandler(&btnMainEventHandler);
  btnMain.setText("Main");
  btnMain.setDebounce(200);
  btnMain.init();

  btnDial.setSize(80,40);
  btnDial.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnDial.setEventHandler(&btnDialEventHandler);
  btnDial.setText("Dial");
  btnDial.setDebounce(200);
  btnDial.init();
  
  btnClearTerminal.setSize(80,40);
  btnClearTerminal.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnClearTerminal.setEventHandler(&btnClearTerminalEventHandler);
  btnClearTerminal.setText("Clear");
  btnClearTerminal.setDebounce(200);
  btnClearTerminal.init();

  numkey.setSize(120,180);
  numkey.setColors(GRAY1,BLACK,WHITE);
  numkey.init();
  numkey.setNumkeyEventHandler(&numkeyEventHandler);
  numkey.setDebounce(100);
  
  header.setText("Main Screen");
  terminal.setColors(WHITE,GRAY2,GRAY1);
    
  canvas.add(&header,0,0);
  canvas.add(&btnMain,0,230);
  canvas.add(&btnDial,80,230);
  canvas.add(&btnClearTerminal,160,230);  
  canvas.add(&terminal,0,270);
  canvas.setScreen(&screen_main);
}

//==================================
// LOOP
//==================================
void loop() {
  // Scan for touch events
  // Every widget is notified and passed
  // the coordinates of the event. 
  // The corresponding widget invokes it's event
  // handler to react to the event.
  canvas.scan();
}
