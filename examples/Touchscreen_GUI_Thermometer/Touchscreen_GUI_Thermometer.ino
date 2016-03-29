/**************************************************************
 *
 *  Touchscreen thermometer
 *
 *  This example demonstrates the asymetrical button grid
 *  configuration. In addition it also demonstrates how
 *  to handle a dial in an application with numkey as a pop-up.
 *
 *  Author: Jose Rullan
 *	Date: 03/27/2016
 *	Release Version: 0.1 beta
 *
 *	License: Whatever only requires to give credit to original 
 *	author (i.e. Jose Rullan)
 *
 *************************************************************/

// Required includes 
#include <SPI.h>
#include <math.h>

// TouchscreenGUI includes
#include <Dial.h>
#include <Gauge.h>
#include <Numkey.h>
#include <Buttongrid.h>
#include <Trend.h>

// Create the objects
//==========================================
Canvas canvas = Canvas(); // Memory used: (storage/ram: 1,676/36)  3,372/228
Dial dial = Dial();       // Memory used: (storage/ram: 4,760/64)  11756/355
Numkey numkey = Numkey(); // Memory used: (storage/ram: 2,370/59)  16,080/503
Buttongrid grid = Buttongrid(3,1);
Buttongrid grid2 = Buttongrid(1,3);
Trend trend = Trend();

// Global variables
// If you need global variables in your program put them here,
// before the setup() routine.
#define LED1 10
#define LED2 2

#define TMP_PIN A4
#define tempUpdate 1000
#define trendUpdate 5000

float tempF;
long lastUpdate,lastTrendUpdate;

void setup() {
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH);
  canvas.init(TFT_LANDSCAPE);

  delay(1000);
  //Configure the widgets
  //=========================================  
    trend.setSize(100,50);
    trend.setColors(GRAY2,GREEN,GRAY1);
    trend.setLimits(60,80,100);
    trend.setHiLimit(85,RED);
    trend.setLowLimit(75,BLUE);
    trend.init();
    
    dial.setSize(50);
    dial.setColors(GRAY2,YELLOW,GRAY1);
    dial.setLimits(60,80,100);
    dial.init();
    dial.setHiLimit(90,RED);
    dial.setLowLimit(75,BLUE);

    grid.setSize(80,90);
    grid.setColors(GRAY2,BLACK,GRAY1);
    grid.setEventHandler(&gridEventHandler);
    grid.setName(1,"HI");
    grid.setName(2,"SP");
    grid.setName(3,"Low");
    grid.init();
    
    grid2.setSize(120,40);
    grid2.setColors(GRAY2,BLACK,GRAY1);
    grid2.setEventHandler(&gridEventHandler);
    grid2.init();
    
    numkey.setSize(120,180);
    numkey.setColors(GRAY1,BLACK,WHITE);
    numkey.init();
    numkey.setNumkeyEventHandler(&numkeyEventHandler);
    numkey.setDebounce(100);
    
  // Add widgets to canvas
  // (Use layout template for coordinates)
  //=========================================
  canvas.add(&dial,75,100);
  canvas.add(&grid,200,50);
  //canvas.add(&grid2,25,190);
  canvas.add(&trend,20,180);

  //while(1);

  
  //Numkey Notes:
  //The numkey widget is meant to be a pop-up
  //kind of widget. So it is added to the canvas 
  //when it is required. Upon pressing the = sign
  //it will call it's event handler and after that
  //it will automatically be removed from the canvas.
  //To prevent this behavior then set
  //	numkey.isPopUp = false; (By default = true)
	//See buttonEventHandler below for an example.
  
  tempF = getTempF(TMP_PIN);
  dial.setCV((int) tempF);
}


void loop() {
  int lastTemp = (int)tempF;
  tempF = getTempF(TMP_PIN);

  if(millis() - lastTrendUpdate > trendUpdate){
    trend.addValue((uint8_t) tempF);
    lastTrendUpdate = millis();
  }
  
  if((int)tempF != lastTemp){
    if(millis()-lastUpdate > tempUpdate){
      
      dial.setCV((int) tempF);
      
      
      if((unsigned int)tempF > dial.hiLimit){
        digitalWrite(LED2,HIGH);
      }else{
        digitalWrite(LED2,LOW);
      }
      
      if((unsigned int)tempF < dial.lowLimit){
        digitalWrite(LED1,HIGH);
      }else{
        digitalWrite(LED1,LOW);
      }
      
      lastUpdate = millis();
    }
  }
  canvas.scan();
}


// Event Handler Functions for buttons
//==========================================
void gridEventHandler(Buttongrid* grid, unsigned char num){
  Serial.print("Button grid number pressed: ");
  Serial.println(num);
  switch(num){
    case 1:
      numkey.setNum(dial.hiLimit);
      break;
    case 2:
      numkey.setNum(dial.setpoint);
      break;
    case 3:
      numkey.setNum(dial.lowLimit);
      break;
    default:
      break;
  }
  canvas.add(&numkey, 80,0);
  dial.visible = false; // do not redraw dial while numkey is shown
  return;
}

void numkeyEventHandler(Numkey* nk){
	if(nk->getTextSize()>0){
    switch(grid.lastPressed){
      case 1:
        dial.hiLimit = nk->getNum();
        break;
      case 2:
        dial.setpoint = nk->getNum();
        break;
      case 3:
        dial.lowLimit = nk->getNum();
        break;
      default:
        break;
    }
  }

  dial.visible = true; // allow dial to redraw again
  
  //Manually pop-up the numkey and force a Dial update
  //Required because normal "autoremove" behavior of numkey occurs "after" this
  //event handling routine finishes, and normal dial's update behavior does not
  //update the value if the previous value and the current value are the same.
  canvas.pop();
  dial.forceUpdate();
}


// Utilities for temperature calculations
//==========================================
float getTempC(int PIN){
  int a = analogRead(PIN);
  float resistance;
  int B = 3975; //thermistor constant

  resistance = (float) (1023-a)*10000/a;
  
  return 1/(log(resistance/10000)/B + 1/298.15) - 273.15;
}

float getTempF(int PIN){
  return getTempC(PIN)*9.0/5.0+32.0;
}
