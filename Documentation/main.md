# Touchscreen_GUI Documentation

The purpose of the library is to enable the use of the Seeedstudio TouchShield v2.0 for HMI control applications. The library provides a subset of widgets that are useful to represent a system's status and interact with the system through several input widgets. 

The architecture of the library consists of one Canvas and several Widgets. The canvas provides the layout, orientation and positioning of widgets, and also monitors the Touchscreen for any touch events. The following figure shows an example of the capabilities of the library.

![Example 1](http://i1181.photobucket.com/albums/x437/jerullan/touchscreen_gui/example1_zpsx5rzp4px.png)

This is another example.

![Example 2](http://i1181.photobucket.com/albums/x437/jerullan/touchscreen_gui/example2_zpscdvnpjw4.png)

The widgets can be classified as input widgets and output widgets. An input widget allows the user to enter a value or activate a function in the Arduino. 

## Input Widgets
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

`Button myButton = Button(10,GRAY1,BLACK,GRAY2);`

This syntax tells the compiler to create a round button with a radius of 10, and the background color GRAY1, the foreground color BLACK and the border color GRAY2.

![Button Configuration](http://i1181.photobucket.com/albums/x437/jerullan/touchscreen_gui/button_conf_zpsni9kaj7m.png)

You can also configure a rectangular button using two dimensions:

`Button myButton = Button(80,40,GRAY1,BLACK,GRAY2);`

This syntax will create a rectangular button with 80 pixels width and 40 pixels height.

More configurations:

    button.setSize(80,40);                       //sets the size of a rectangular button
    button.setColors(GRAY1,BLACK,WHITE);         //sets the bgColor, fgColor and borderColor
    button.setText("Change");                    //sets the text displayed on the button
    button.setEventHandler(&buttonEventHandler); //sets the event handler for the button
    button.init();                               //Initializes the button (required)
    button.setDebounce(100);                     //Change the debouncing from 200 to 100 mS


**Numkey**

The Numkey widget is a special button. It creates a numerical entry pad to enter a value. When the = sign is pressed a function in your Arduino sketch is invoked to allow you to use the value for whatever purpose.

![Numerical Input](http://i1181.photobucket.com/albums/x437/jerullan/touchscreen_gui/numkey_zpsmi8x7kdc.png)


## Output Widgets
The library provides three output widgets: Display, Dial and Gauge. 

**Display** 

The Display widget is used to show a value or a text on the Canvas. 

**Dial**

The Dial widget is used to show a value in relation with a scale. It also provides configurable limits to determine whether the current value is above or below some threshold. These limits are Low limit, Setpoint, High limit. The Low and Hi limits are configured by providing a value and a desired color for the bezel of the Dial. 

**Gauge**

The Gauge is similar to the dial but draws a vertical bar instead. It also has configurable limits and a scale. The bar color changes depending on the relation of the current value with regards to the Low and Hi limits.
