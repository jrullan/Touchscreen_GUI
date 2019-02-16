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

#include <SPI.h>
#include <DHTesp.h>
#include <neotimer.h>

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
#define DHT_PIN 2 // Wemos D1 Mini D4

// For MH-ET Live esp32 MiniKit (ESP32)
#define TFT_CS 26 // esp32 MiniKit D0
#define TFT_DS 5  // esp32 MiniKit D8
#define TS_CS 17  // esp32 MiniKit D3
#define DHT_PIN 16// esp32 MiniKit D4

DHTesp dht;

Canvas_XPT2046 canvas = Canvas_XPT2046(TFT_PORTRAIT,BLACK,TFT_CS,TFT_DS,TS_CS);
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
IconButton btnSliderSimulate = IconButton(60,30,slider_off,slider_on);

Neotimer readTempTimer = Neotimer(5000);
Neotimer simTimer = Neotimer(10);
bool direction = true;
float temperature;
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
  //canvas.add(&numkey,60,50);
  header.setText("Dial");
  canvas.setScreen(&screen_dial);
  terminal.clear();
  terminal.print("Dial reprents a value in a range");
  terminal.print("Press the + and - buttons");
  terminal.print("To change it's value");
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
      if(nk->contents->text[i] != password[i]) match = false; 
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

void btnButtonsEventHandler(Button* btn){
  header.setText("Buttons");
  canvas.setScreen(&screen_buttons);
  terminal.clear();
  terminal.print("Icon buttons use images to");
  terminal.print("represent the button state");
  terminal.print("Press a button to see it's state");
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
  dht.setup(DHT_PIN, DHTesp::DHT11);
  
  canvas.init();

  // ===== MAIN SCREEN =====
  screen_main.bgColor = 0xDFE;
  title.setSize(240,40);
  title.setColors(screen_main.bgColor,GRAY2,screen_main.bgColor);
  title.borderWidth = 0;
  title.setText("GUI DEMO",false);
  screen_main.add(&title,0,screen_main.h/2-FONT_Y*title.fontSize);

  // ===== SCREEN TWO =====
  btnPlus.setText("+");
  btnPlus.setEventHandler(&btnPlusEventHandler);
  btnPlus.setDebounce(25);  

  btnMinus.setText("-");
  btnMinus.setEventHandler(&btnMinusEventHandler);
  btnMinus.setDebounce(25);  

  btnSliderSimulate.setEventHandler([](Button* btn){
    btn->update();
  });
  btnSliderSimulate.transparentColor = BLACK;  
  btnSliderSimulate.setLabel("Sim");

  dial.init();
  dial.setSize(50);
  dial.borderWidth = 5;
  dial.setColors(WHITE,GRAY2,GRAY2);
  dial.setLimits(60,70,90);
  dial.setHiLimit(75,RED);
  dial.setSP(70,BLUE);
  dial.setLowLimit(70,BLUE);
  dial.setCV(72,false);

  screen_dial.bgColor = screen_main.bgColor;//0xf80f;
  screen_dial.add(&dial,100,screen_dial.h/2);
  screen_dial.add(&btnPlus,160,screen_dial.h/2 - dial.radius);
  screen_dial.add(&btnMinus,160,screen_dial.h/2 + dial.radius - btnMinus.h);
  screen_dial.add(&btnSliderSimulate,100,screen_dial.h/2 + dial.radius + 10);

  // ===== SCREEN THREE - BUTTONS  =====  
  screen_buttons.bgColor = 0xDFE;

  btnBulb.setEventHandler(&btnIconEventHandler);
  btnBulb.transparentColor = BLACK;

  btnSlider.setEventHandler(&btnIconEventHandler);
  btnSlider.transparentColor = BLACK;  
  btnSlider.setLabel("Temp");

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


  /*
  numkey.setSize(120,180);
  numkey.setColors(GRAY1,BLACK,WHITE);
  //numkey.init();
  numkey.setEventHandler(&numkeyEventHandler);
  numkey.setDebounce(100);
  */
  
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

  if(canvas.currentScreen == &screen_dial && btnSliderSimulate.touched && simTimer.repeat()){
    if(dial.getCV() >= dial.scaleMax || dial.getCV() <= dial.scaleMin){
      direction = !direction;
      if(direction){
        terminal.print("Now pressing +",CYAN);
      }else{
        terminal.print("Now pressing -",CYAN);
      }
    }
    
    if(direction){
      btnPlusEventHandler(&btnPlus);
    }else{
      btnMinusEventHandler(&btnMinus);   
    }    
  }
  
  if(btnSlider.touched && readTempTimer.repeat()){
    setTemperature();
  }
  
}

void setTemperature(){
  temperature = dht.getTemperature();
  temperature = dht.toFahrenheit(temperature);
  dial.setCV(temperature);
  String temp = "Temperature = ";
  temp += temperature;
  temp += " F";
  terminal.print(temp);
}
