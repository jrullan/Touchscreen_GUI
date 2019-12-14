/* 
 * The demo showcases the following features:
 * Display widgets used to show a text
 * Buttons and Round Buttons
 * Multiple Screens
 * Use of the Numkey widget as a password entry
 * Dial indicator
 * Terminal widget useful for on-screen information
 * 
 * Created by: Jose Rullan
 * Date: February 16, 2019
 */

#include <Canvas_XPT2046.h>
#include <Screen.h>
#include <Terminal.h>
#include <Display.h>
#include <Button.h>
#include <Dial.h>
#include <Numkey.h>
#include <IconButton.h>
#include <icons.h>

// For Wemos Mini D1 (ESP8266)
#define TFT_CS 16 // Wemos D1 Mini D0
#define TFT_DS 15 // Wemos D1 Mini D8
#define TS_CS 0   // Wemos D1 Mini D3

// For MH-ET Live esp32 MiniKit (ESP32)
#define TFT_CS 26 // esp32 MiniKit D0
#define TFT_DS 5  // esp32 MiniKit D8
#define TS_CS 17  // esp32 MiniKit D3

#define BG_COLOR 0xDFE  //Background color (light blue)
Canvas_XPT2046 canvas = Canvas_XPT2046(TFT_PORTRAIT,BG_COLOR,TFT_CS,TFT_DS,TS_CS);
Display header = Display(240,40,BLUE,WHITE,WHITE,20);
Display title = Display(20);
Terminal terminal = Terminal(240,50,TERMINAL_SCROLL_UP);
Numkey numkey = Numkey();
Button btnMain = Button();
Button btnDial = Button();
Button btnButtons = Button();
Screen screen_main = Screen(&canvas,0,40,240,190);
Screen screen_dial = Screen(&canvas,0,40,240,190);
Screen screen_buttons = Screen(&canvas,0,40,240,190);
Dial dial = Dial();
Button btnPlus = Button(20,GRAY1,WHITE,ILI9341_LIGHTGREY);  //Initialization version for round buttons
Button btnMinus = Button(20,GRAY1,WHITE,ILI9341_LIGHTGREY); //Initialization version for round buttons
IconButton btnBulb = IconButton(50,50,lightbulb_off,lightbulb_on);
IconButton btnSlider = IconButton(60,30,slider_off,slider_on);

const char increment = 1;

//==================================
// EVENT HANDLING ROUTINES
//==================================

void btnIconEventHandler(Button* btn){
  if(btn == &btnBulb){
    terminal.print((btn->touched)?"Bulb pressed":"Bulb unpressed",YELLOW);
  }
  if(btn == &btnSlider){
    terminal.print((btn->touched)?"Slider On":"Slider Off",GREEN);
  }
  btn->update();
}

void btnMainEventHandler(Button* btn){
  header.setText("Main");
  canvas.setScreen(&screen_main);
  terminal.clear();
  welcomeMessage();
}

void btnDialEventHandler(Button* btn){
  // Show the numkey to enter a password
  // The numkey event handler will verify the password
  // and change the screen if the password is correct
  terminal.clear();
  terminal.print("Enter the numeric password:");
  terminal.print("1234",GREEN);
  canvas.add(&numkey,60,45);
}

void btnButtonsEventHandler(Button* btn){
  header.setText("Buttons");
  canvas.setScreen(&screen_buttons);
  terminal.clear();
  terminal.print("Icon buttons use images to");
  terminal.print("represent the button state");
  terminal.print("Press a button to see it's state");
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
    Serial.println(nk->getText());
    for(int i=0;i<nk->getTextSize();i++){
      if(nk->getText()[i] != password[i]) match = false; 
    }
    if(match){
      header.setText("Dial");
      canvas.setScreen(&screen_dial);
      terminal.clear();
      terminal.print("Dial reprents a value in a range");
      terminal.print("Press the + and - buttons");
      terminal.print("To change it's value");
      nk->clear();
      return;
    }
  }
  terminal.print("Wrong password entered",RED);
  nk->clear();
}

void welcomeMessage(){
  terminal.print("Welcome to Touchscreen GUI demo",YELLOW);
  terminal.print("This demo shows up some features",GREEN);
  terminal.print("of the library. Enjoy!");  
}

//==================================
// SETUP
//==================================
void guiSetup(){
  // ===== MAIN SCREEN =====
  screen_main.bgColor = BG_COLOR;
  title.setSize(240,40);
  title.setColors(screen_main.bgColor,GRAY2,screen_main.bgColor);
  title.borderWidth = 0;
  title.setText("GUI DEMO",false);
  screen_main.add(&title,0,screen_main.h/2-FONT_Y*title.fontSize);

  // ===== SCREEN TWO =====
  screen_dial.bgColor = BG_COLOR;

  btnPlus.setText("+");
  btnPlus.setEventHandler(&btnPlusEventHandler);
  btnPlus.setDebounce(25);  

  btnMinus.setText("-");
  btnMinus.setEventHandler(&btnMinusEventHandler);
  btnMinus.setDebounce(25);  

  dial.init();
  dial.setSize(50);
  dial.borderWidth = 5;
  dial.setColors(WHITE,GRAY2,GRAY2);
  dial.setLimits(60,70,90);
  dial.setHiLimit(75,RED);
  dial.setSP(70,BLUE);
  dial.setLowLimit(70,BLUE);
  dial.setCV(72,false);

  screen_dial.add(&dial,100,screen_dial.h/2);
  screen_dial.add(&btnPlus,160,screen_dial.h/2 - dial.radius);
  screen_dial.add(&btnMinus,160,screen_dial.h/2 + dial.radius - btnMinus.h);

  // ===== SCREEN THREE - BUTTONS  =====  
  screen_buttons.bgColor = BG_COLOR;

  btnBulb.setEventHandler(&btnIconEventHandler);
  btnBulb.transparentColor = BLACK;

  btnSlider.setEventHandler(&btnIconEventHandler);
  btnSlider.transparentColor = BLACK;

  screen_buttons.add(&btnBulb,95,50);
  screen_buttons.add(&btnSlider,90,110);

  // ===== CANVAS GENERAL ITEMS  =====  
  btnMain.setSize(80,40);
  btnMain.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnMain.cornerRadius = 8;
  btnMain.setEventHandler(&btnMainEventHandler);
  btnMain.setText("Main");
  btnMain.setDebounce(200);

  btnDial.setSize(80,40);
  btnDial.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnDial.cornerRadius = 8;
  btnDial.setEventHandler(&btnDialEventHandler);
  btnDial.setText("Dial");
  btnDial.setDebounce(200);

  btnButtons.setSize(80,40);
  btnButtons.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnButtons.cornerRadius = 8;
  btnButtons.setEventHandler(&btnButtonsEventHandler);
  btnButtons.setText("Btns");
  btnButtons.setDebounce(200);

  numkey.setSize(120,180);
  numkey.setColors(GRAY1,BLACK,WHITE);
  numkey.init();
  numkey.setEventHandler(&numkeyEventHandler);
  numkey.setDebounce(100);

  header.setText("Main Screen",false);
  terminal.setColors(BLACK,WHITE,WHITE);

  canvas.add(&header,0,0);
  canvas.add(&btnMain,0,230);
  canvas.add(&btnDial,80,230);
  canvas.add(&btnButtons,160,230);  
  canvas.add(&terminal,0,270);
  canvas.setScreen(&screen_main);  

  welcomeMessage();
}

void setup() {
  Serial.begin(115200);
  guiSetup();
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