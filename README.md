# Touchscreen_GUI
GUI Library for Seeedstudio Touchscreen Shield v2.0


See the wiki: https://github.com/jrullan/Touchscreen_GUI/wiki

This library was developed for Seeedstudio
TouchShield v2.0, based on their TFTv2 and SeeedTouchscreen
libraries. This library is intended to provide a minimal
but useful collection of reusable widgets that can be used
to create a Graphical User Interface (GUI) for any Arduino 
project.

Author: Jose Rullan
Release Date: 03/28/2015  -- Arduino Day 2015!!!
Release Version: 0.1 beta

License: MIT


# Introduction

The purpose of the library is to enable the use of the Seeedstudio TouchShield v2.0 for HMI control applications. The library provides a subset of widgets that are useful to represent a system's status and interact with the system through several input widgets.

## Hardware Architecture

There are three hardware architecture components supported by the library. 

1. MCU architecture support - **ATmel AVR**, **SAM**, **STM32**, **ESP8266**, **ESP32**, etc
2. TFT chipset - **ILI9341**
3. Touch chipset - **Analog resistive**, **STMPE610**, **XPT2046**, **FT6206**

## Software Architecture

The architecture of the library consists of one **Canvas**, a **Touch object**, a **Point object**, **Screens** and **Widgets**. The canvas provides the layout, orientation and positioning of Screens and Widgets, and also monitors the Touch object for any touch events. If a touch event is detected then it returns a Point object that represents the coordinates of the touch event in relation with the Widgets drawn on the Canvas. 

The class architecture
![Class Architecture](https://raw.githubusercontent.com/jrullan/Touchscreen_GUI/master/images/TouchscreenGUI Architecture 2.PNG)



* * *

The two main classes that make the UI are:
1. Canvas
2. Widgets

# Software Architecture (in-depth)

## Canvas
The canvas represents the total screen area. An application will have only one canvas and all UI elements will be placed on it. 

The main responsibilities of the canvas are:
1. Provide the context for all UI elements (***widgets***)
2. Determine the orientation of the screen and hence the width and height properties
3. Determine the background color of the screen
4. Be responsible for handling the touch events of the screen
5. Notify all widgets when an event has been detected and provide them with necessary information such as the coordinates of the event.

## Widgets
The widgets are the UI elements themselves. Examples of widgets are buttons, dials, text etc. Some widgets will only show something on the UI, while others will "react" to the user through the touchscreen. These widgets that will "react" to the touch events are called ***Input Widgets***. 

### Input Widgets
Some of the widgets will be  to the touch of the screen. 

The library provides two input widgets: Button and Numkey. Because input widgets are meant to give the Arduino a command or a parameter, these need an event handler procedure that will be executed whenever they detect that the user has pressed them.

To provide this function you must pass the "address" (**&**) of the function during the 'setup'. Example:

`myButton.setEventHandler(&eventHandlerFunction);`

**eventHandlerFunction** is a function that must be defined in your sketch and should be like this:

`void eventHandlerFunction(Button* btn){...`

This function will be called once every time that the button is pressed. Inside the function you can use **btn** as a pointer to the button that was pressed, allowing you to manipulate its appearance for example. 

`btn->bgColor = GREEN; //Change background of button to green when pressed`

Also, both have a debouncing feature set by default to 200 mS. This time can be set using the setDebounce function:

`myButton.setDebounce(100);`


**Button**

The button widget activates an event handler function in the Arduino sketch when a touch event is detected. The buttons can be rectangular or round. 

![Buttons](http://i1181.photobucket.com/albums/x437/jerullan/touchscreen_gui/button_zpsqklkvm5s.png)

To configure a button as a round button, you must create the button using the syntax: 

```c
Button myButton = Button(10,GRAY1,BLACK,GRAY2);
```

This syntax tells the compiler to create a round button with a radius of 10, and the background color GRAY1, the foreground color BLACK and the border color GRAY2.

![Button Configuration](http://i1181.photobucket.com/albums/x437/jerullan/touchscreen_gui/button_conf_zpsni9kaj7m.png)

You can also configure a rectangular button using two dimensions:

```c
Button myButton = Button(80,40,GRAY1,BLACK,GRAY2);
```

This syntax will create a rectangular button with 80 pixels width and 40 pixels height.

More configurations:

```c
button.setSize(80,40);                       //sets the size of a rectangular button
button.setColors(GRAY1,BLACK,WHITE);         //sets the bgColor, fgColor and borderColor
button.setText("Change");                    //sets the text displayed on the button
button.setEventHandler(&buttonEventHandler); //sets the event handler for the button
button.init();                               //Initializes the button (required)
button.setDebounce(100);                     //Change the debouncing from 200 to 100 mS
```

**Numkey**

The Numkey widget is a special button. It creates a numerical entry pad to enter a value. When the = sign is pressed a function in your Arduino sketch is invoked to allow you to use the value for whatever purpose.

![Numerical Input](http://i1181.photobucket.com/albums/x437/jerullan/touchscreen_gui/numkey_zpsmi8x7kdc.png)


### Output Widgets
The library provides three output widgets: Display, Dial and Gauge. 

**Display** 

The Display widget is used to show a value or a text on the Canvas. 

**Dial**

The Dial widget is used to show a value in relation with a scale. It also provides configurable limits to determine whether the current value is above or below some threshold. These limits are Low limit, Setpoint, High limit. The Low and Hi limits are configured by providing a value and a desired color for the bezel of the Dial. 

**Gauge**

The Gauge is similar to the dial but draws a vertical bar instead. It also has configurable limits and a scale. The bar color changes depending on the relation of the current value with regards to the Low and Hi limits.

# Canvas, screens and widgets

## Canvas

The **Canvas** is the main class responsible for drawing the **widgets** and for detecting the touch events on the screen. The canvas has a ***widgets*** collection where it stores a pointer to each widget added to the canvas. The canvas main method is the **`scan()`** method. Typically in the `loop()` function of your Arduino sketch this is the only function called. 

```c
void loop(){
    canvas.scan();
}
```

When an event is detected during the **`scan()`** method it calls the **`widget.checkTouch(point)`** method on every widget of the collection passing as an argument the **Point** where the event was detected. The checkTouch(point) method checks if the coordinates of the point coincides with the coordinates of any ***hot spot*** within the widget's coordinates. If it does, then the widget calls its **`eventHandler(this)`** function. This function is a pointer to a function defined in the sketch and set during configuration by calling the widget's **`setEventHandler(&functionName)`** function. 

**`functionName`** is the actual user code that handles the event generated by the widget. The function must abide to the signature of the specific widget because the widget might need to send additional information. The following is an example of a sketch that uses a button to toggle a digital output (D5) upon each press. (Button has a toggleable attribute called touched that toggles on each new event between true and false):

```c
void btnEventHandler(Button* btn)
{
    //Do something after the button has been pressed
    //Note that btn is a reference to the button that triggered this event.
    if(btn->touched){
        btn->bgColor = GREEN;
        digitalWrite(D5,HIGH);
    }else{
        btn->bgColor = GRAY1;
        digitalWrite(D5,LOW);
    }
}

void setup(){
    //....
    button.setEventHandler(&btnEventHandler);
    button.init();
    canvas.add(&button,100,100);
    //....
}

void loop(){
    canvas.scan();
}
```

The signature of a function that can be used as an event handler for a **Button** widget is **`void functionName(Button* btn)`**. Most widgets that accept events (with property isButton set to true) will have a similar signature. The following are the signature of the typical widgets:

```c
void functionName(Button* btn){}
void functionName(Numkey* nk){}
void functionName(Popup* p,unsigned char val){}
void functionName(Buttongrid* bg, uint8_t val){}
```

> Note: Because we are using references to other functions we need to use the ***&*** symbol before the name of the function. Officially, Arduino discourages the use of libraries that requires this type of references, but to provide the functionality and versatility of this library we need to pass addresses extensively throughout the library and sketch.

## Screens

Besides widgets a canvas can have a **Screen** object set in its **`canvas.currentScreen`** attribute. The **Screen** object represents a screen or page of the application. The screen is a group of widgets that must be presented together. You can define multiple screens and each screen should have its own widgets. The screen keeps track of its widgets in the ***widgets*** attribute and takes on the responsibility of drawing the widgets and passing down the events detected by the canvas.

![Canvas, screens and widgets](https://raw.githubusercontent.com/jrullan/Touchscreen_GUI/master/images/canvas_screen.png)

When a touch event is detected it will be first routed to the canvas widgets, then it will be allowed to go to the **currentScreen**. This means that the canvas widgets have precedence over the screen widgets when handling a touch event. If you are using a screen, then widgets added to the canvas will be drawn on top of the screen widgets and will first receive the touch event. This design provides a practical way to define navigation buttons and other widgets common to all screens that can be accessible from any screen. For example you can have a **Popup** widget or a **Numkey** widget defined in the canvas and you can reuse it in every screen you need to use it without creating a new object that will consume precious memory. A way to do this is through special **`show(Button* btn)`** methods in **Popup** and **Numkey**. Lets see an example.

```c
Canvas canvas = Canvas();
Numkey numkey = Numkey(120,180,GRAY1,BLACK,WHITE);

Screen myScreen = Screen(&canvas);
Button screenButton = Button(80,40,BLACK,GREEN,GRAY1);

void setup(){
  // Canvas Setup
  canvas.init(TFT_LANDSCAPE);
  canvas.setDebounce(100); 

  numkey.init();
  numkey.visible = false;
  numkey.autoremove = false; //Disable autoremove because we are using screens
  numkey.setNumkeyEventHandler(&numkeyEventHandler);
  numkey.setDebounce(400);

  canvas.add(&numkey,100,30);

  // Screen widget 
  screenButton.setText("100");
  screenButton.setEventHandler(&screenButtonEventHandler);
  screenButton.init();
  myScreen.add(&screenButton,120,50);

  canvas.setScreen(&myScreen); // sets it as the current screen
}

// Event handling functions:
void screenButtonEventHandler(Button* btn){
  if(numkey.entry == NULL){
    numkey.show(btn);
  }else{

    //Your code goes in here
    if(numkey.getTextSize() > 0){
      btn->setNum(numkey.entry);
    }
    //

    numkey.reset();
  }
}

void numkeyEventHandler(Numkey* nk){
  nk->hide(); 
  nk->targetEventHandler(); 
}
```

# Demo Application
The demo application is a complete GUI application that shows the features of the library and some of the implemented widgets. This demo demonstrates an application consisting of a title, a screen, navigation buttons, a terminal, a Dial widget and Icon Buttons. The following diagram represents the application components and their relationship.

![](https://i1181.photobucket.com/albums/x437/jerullan/touchscreen_gui/Touchscreen_GUI%20Demo_zpsip1d1k4z.png)

We need to first include the canvas library depending on the type of touchscreen we are going to use. The canvas is the main object of the library and all other widgets are added to a canvas or a screen. The canvas has only one current screen object at a time.

```c++
/* You can use either of these Canvas child classes, depending on your type of TFT touchscreen. */
//#include <Canvas_SEEEDTOUCH.h>
//#include <Canvas_FT6206.h>
#include <Canvas_STMPE610.h>
```
Once the canvas has been included, you need to include the specific widgets you will use in your application. From the picture above, we see we will need the following widgets:
```c++
#include <Screen.h>
#include <Terminal.h>
#include <Display.h>
#include <Button.h>
#include <Dial.h>
#include <IconButton.h>
#include <icons.h>
```
These are all of the widgets we need. Now we will create the instances. 
Many of the constructors have also some initialization parameters.
```c++
// Use either of these three and
// its corresponding include above
//Canvas_SEEEDTOUCH canvas = Canvas_SEEEDTOUCH(TFT_PORTRAIT,BLACK);
//Canvas_FT6206 canvas = Canvas_FT6206(TFT_PORTRAIT,BLACK);
Canvas_STMPE610 canvas = Canvas_STMPE610(TFT_PORTRAIT,BLACK);

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
```

Most of the widgets have events callback routines that automatically invoked when an event is detected by a widget's checktouch method.
```c++
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
  canvas.add(&numkey,60,50);
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
```

Each of the GUI components must be configured. We put all the configuration code in the following function guiSetup():

```c++
void guiSetup(){
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
  dial.setCV(72,false);

  screen_dial.bgColor = screen_main.bgColor;//0xf80f;
  screen_dial.add(&dial,100,screen_dial.h/2);
  screen_dial.add(&btnPlus,160,screen_dial.h/2 - dial.radius);
  screen_dial.add(&btnMinus,160,screen_dial.h/2 + dial.radius - btnMinus.h);

  // ===== SCREEN THREE - BUTTONS  =====  
  screen_buttons.bgColor = 0xDFE;

  btnBulb.setEventHandler(&btnIconEventHandler);
  btnBulb.transparentColor = BLACK;
  btnBulb.init();

  btnSlider.setEventHandler(&btnIconEventHandler);
  btnSlider.transparentColor = BLACK;
  btnSlider.init();

  screen_buttons.add(&btnBulb,95,50);
  screen_buttons.add(&btnSlider,90,110);

  // ===== CANVAS GENERAL ITEMS  =====  
  btnMain.setSize(80,40);
  btnMain.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnMain.cornerRadius = 8;
  btnMain.setEventHandler(&btnMainEventHandler);
  btnMain.setText("Main");
  btnMain.setDebounce(200);
  btnMain.init();

  btnDial.setSize(80,40);
  btnDial.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnDial.cornerRadius = 8;
  btnDial.setEventHandler(&btnDialEventHandler);
  btnDial.setText("Dial");
  btnDial.setDebounce(200);
  btnDial.init();
  
  btnButtons.setSize(80,40);
  btnButtons.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnButtons.cornerRadius = 8;
  btnButtons.setEventHandler(&btnButtonsEventHandler);
  btnButtons.setText("Btns");
  btnButtons.setDebounce(200);
  btnButtons.init();
  
  numkey.setSize(120,180);
  numkey.setColors(GRAY1,BLACK,WHITE);
  numkey.init();
  numkey.setEventHandler(&numkeyEventHandler);
  numkey.setDebounce(100);
  
  header.setText("Main Screen",false);
  terminal.setColors(BLACK,WHITE,WHITE);
    
  canvas.add(&header,0,0,false);
  canvas.add(&btnMain,0,230,false);
  canvas.add(&btnDial,80,230,false);
  canvas.add(&btnButtons,160,230,false);  
  canvas.add(&terminal,0,270,false);
  canvas.setScreen(&screen_main);  

  welcomeMessage();
}
```
Once all configuration has been done we simply call it in our setup() function. Then in the loop section we just need to call canvas.scan() method which checks for a touch event to occur and if it does it sends the event to every widget until one of them handles the event.

```c++

//==================================
// SETUP
//==================================
void setup() {
  Serial.begin(115200);
  guiSetup();
}

//==================================
// LOOP
//==================================
void loop() {
  canvas.scan();
}

```

# Widgets
## Button

```c++
#include <Canvas_XPT2046.h>
#include <Button.h>

// For MH-ET Live esp32 MiniKit (ESP32)
#define TFT_CS 26 // esp32 MiniKit D0
#define TFT_DS 5  // esp32 MiniKit D8
#define TS_CS 17  // esp32 MiniKit D3

Canvas_XPT2046 canvas = Canvas_XPT2046(TFT_PORTRAIT,BLACK,TFT_CS,TFT_DS,TS_CS);

Button button = Button();

void buttonEventHandler(Button* btn){
  Serial.println("Button Pressed");
}

void setup(){
  Serial.begin(115200);

  canvas.init();

  button.setSize(80,40);
  button.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  button.cornerRadius = 8;
  button.setText("A Button");
  button.setDebounce(200);
  button.setEventHandler(&buttonEventHandler);

  canvas.add(&button,0,100);  
}

void loop(){
  canvas.scan();
}    
```
