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
 * 
 * Update: 
 * This particular demo uses global pointers for
 * the Touchscreen_GUI's library object instances.
 * The reason was that when downloaded into an Arduino
 * Mega with the Seeedtouch TFT screen the program crashed
 * upon the declaration of the canvas object.
 * 
 * When it was declared as a pointer instead and was
 * instantiated inside the guiSetup routine, it worked.
 * The only drawback so far is that when compiled the 
 * Global variables use of the dynamic memory dropped about
 * 6%, from 20% with global object instances to 14% with
 * global pointers. I believe that this could be a misrepresentation
 * of the total dynamic memory allocation during runtime,
 * and if so, there is a risk that there could be a crash if that 
 * memory allocation exceeds the real available memory in runtime.
 * 
 */

#include <Canvas_SEEEDTOUCH.h>
//#include <Canvas_XPT2046.h>

#include <Screen.h>
#include <Terminal.h>
#include <Display.h>
#include <Button.h>
#include <Slider.h>
#include <Dial.h>
#include <Numkey.h>
#include <IconButton.h>
#include <icons.h>
#include <neotimer.h>


#define BG_COLOR 0xDFE  //Background color (light blue)

Canvas_SEEEDTOUCH *canvas;
Screen *screen_main;
Screen *screen_dial;
Screen *screen_buttons;
Display *header;
Display *title;
Terminal *terminal;
Numkey *numkey;
Button *btnMain;
Button *btnDial;
Button *btnButtons;
Dial *dial;
Slider *slider;
IconButton *btnBulb;
IconButton *btnSlider;

Neotimer dialTimer = Neotimer(5000);
Neotimer incrementTimer = Neotimer(10);

bool passwordCorrect = false;
int8_t increment = 2;

//==================================
// EVENT HANDLING ROUTINES
//==================================

void btnIconEventHandler(Button* btn){
  if(btn == btnBulb){
    terminal->print((btn->touched)?"Bulb pressed":"Bulb unpressed",YELLOW);
  }
  if(btn == btnSlider){
    terminal->print((btn->touched)?"Slider On":"Slider Off",GREEN);
  }
  btn->update();
}

void btnMainEventHandler(Button* btn){
  header->setText("Main",true);
  canvas->setScreen(screen_main);
  terminal->clear();
  welcomeMessage();
}

void btnDialEventHandler(Button* btn){
  // Show the numkey to enter a password
  // The numkey event handler will verify the password
  // and change the screen if the password is correct
  if(!passwordCorrect){
    terminal->clear();
    terminal->print("Enter the numeric password:");
    terminal->print("1234",GREEN);
    canvas->add(numkey,60,45);
  }else{
    numkeyEventHandler(numkey);
  }
}

void btnButtonsEventHandler(Button* btn){
  header->setText("Buttons",true);
  canvas->setScreen(screen_buttons);
  terminal->clear();
  terminal->print("Icon buttons use images to");
  terminal->print("represent the button state");
  terminal->print("Press a button to see it's state");
}

void sliderEventHandler(Slider* sld){
	dial->setCV(map(sld->currentValue,0,100,dial->scaleMin,dial->scaleMax));
  dialTimer.reset();
  dialTimer.start();
}

void numkeyEventHandler(Numkey* nk){
  if(passwordCorrect==false){
    char* password = "1234";
    bool match = true;
    if(nk->getTextSize() == Widget::getTextLength(password)){
      Serial.println(nk->getText());
      for(int i=0;i<nk->getTextSize();i++){
        if(nk->getText()[i] != password[i]){
          match = false;
          break; 
        }
      }
    }else{
      match = false;
    }
    if(!match){
      terminal->print("Incorrect Password",RED);
      nk->clear();      
      return;
    }  
    passwordCorrect = true;
    dialTimer.start();
  }

  header->setText("Dial & Slider",true);
  canvas->setScreen(screen_dial);
  terminal->clear();
  terminal->print("Dial reprents a value in a range");
  terminal->print("Touch anywhere on the slider bar");
  terminal->print("To change it's value");
}

void welcomeMessage(){
  terminal->print("Welcome to Touchscreen GUI demo",YELLOW);
  terminal->print("This demo shows up some features",GREEN);
  terminal->print("of the library. Enjoy!");  
}

//==================================
// SETUP
//==================================
void guiSetup(){
  canvas = new Canvas_SEEEDTOUCH(TFT_PORTRAIT,BG_COLOR);
  screen_main = new Screen(canvas,0,40,240,190);
  screen_dial = new Screen(canvas,0,40,240,190);
  screen_buttons = new Screen(canvas,0,40,240,190);
  header = new Display(240,40,BLUE,WHITE,WHITE,20);
  title = new Display(20);
  terminal = new Terminal(240,50,TERMINAL_SCROLL_UP);
  numkey = new Numkey();  
  btnMain = new Button();
  btnDial = new Button();
  btnButtons = new Button();
  dial = new Dial();
  slider = new Slider();
  btnBulb = new IconButton(50,50,lightbulb_off,lightbulb_on);
  btnSlider = new IconButton(60,30,slider_off,slider_on);
  
  // ===== MAIN SCREEN =====
  screen_main->bgColor = BG_COLOR;
  title->setSize(240,40);
  title->setColors(screen_main->bgColor,GRAY2,screen_main->bgColor);
  title->borderWidth = 0;
  title->setText("GUI DEMO",false);
  screen_main->add(title,0,screen_main->h/2-FONT_Y*title->fontSize);

  // ===== SCREEN TWO =====
  screen_dial->bgColor = BG_COLOR;

  // Blue slider elements
  slider->setDebounce(0);
  slider->setSize(40,140);
  slider->setColors(BLACK,GRAY1,WHITE);
  slider->setEventHandler(&sliderEventHandler);
	
  dial->init();
  dial->setSize(50);
  dial->borderWidth = 5;
  dial->setColors(WHITE,GRAY2,GRAY2);
  dial->setLimits(60,70,90);
  dial->setHiLimit(75,RED);
  dial->setSP(70,BLUE);
  dial->setLowLimit(70,BLUE);
  dial->setCV(72,false);

  screen_dial->add(dial,80,screen_dial->h/2);
	screen_dial->add(slider,170,20);
	
  // ===== SCREEN THREE - BUTTONS  =====  
  screen_buttons->bgColor = BG_COLOR;

  btnBulb->setEventHandler(&btnIconEventHandler);
  btnBulb->transparentColor = BLACK;

  btnSlider->setEventHandler(&btnIconEventHandler);
  btnSlider->transparentColor = BLACK;
  btnSlider->fgColor = BLACK;
  btnSlider->labelPos = LABEL_RIGHT;
  btnSlider->setLabel("Simulate");

  screen_buttons->add(btnBulb,95,70);
  screen_buttons->add(btnSlider,10,10);

  // ===== CANVAS GENERAL ITEMS  =====  
  btnMain->setSize(80,40);
  btnMain->setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnMain->cornerRadius = 8;
  btnMain->setEventHandler(&btnMainEventHandler);
  btnMain->setText("Main");
  btnMain->setDebounce(200);

  btnDial->setSize(80,40);
  btnDial->setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnDial->cornerRadius = 8;
  btnDial->setEventHandler(&btnDialEventHandler);
  btnDial->setText("Dial");
  btnDial->setDebounce(200);

  btnButtons->setSize(80,40);
  btnButtons->setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnButtons->cornerRadius = 8;
  btnButtons->setEventHandler(&btnButtonsEventHandler);
  btnButtons->setText("Btns");
  btnButtons->setDebounce(200);

  numkey->setSize(120,180);
  numkey->setColors(GRAY1,BLACK,WHITE);
  numkey->init();
  numkey->setEventHandler(&numkeyEventHandler);
  numkey->setDebounce(100);

  header->setText("Main Screen",false);
  terminal->setColors(BLACK,WHITE,WHITE);

  canvas->add(header,0,0);
  canvas->add(btnMain,0,230);
  canvas->add(btnDial,80,230);
  canvas->add(btnButtons,160,230);  
  canvas->add(terminal,0,270);
  canvas->setScreen(screen_main);  

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
  canvas->scan();

  if(dialTimer.done() && btnSlider->touched){
    if(incrementTimer.repeat()){
      if(slider->currentValue > 100) slider->currentValue = 100;
      if(slider->currentValue < 0) slider->currentValue = 0;
      if(slider->currentValue == 100 || slider->currentValue == 0) increment = increment * -1;
      slider->currentValue = constrain(slider->currentValue + increment, slider->scaleMin, slider->scaleMax);      
      if(canvas->currentScreen == screen_dial) slider->update();
      dial->setCV(map(slider->currentValue,0,100,dial->scaleMin,dial->scaleMax));
    }
  }
}
