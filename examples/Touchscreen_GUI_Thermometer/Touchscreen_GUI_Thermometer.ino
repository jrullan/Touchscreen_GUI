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

// --Constants
#define LED1 10
#define LED2 2
#define BACKLIGHT 7

#define TMP_PIN A4
#define tempUpdate 1000
#define trendUpdate 300000
#define backLightOffDelay 100

Canvas canvas = Canvas(TFT_LANDSCAPE,BLACK);
Dial dial;
Numkey numkey;
Buttongrid grid = Buttongrid(3,1);
Trend trend;

float tempF;
long lastUpdate,lastTrendUpdate, backLightOffUpdate;

void setup() {
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BACKLIGHT,OUTPUT);
  digitalWrite(BACKLIGHT,HIGH);
  backLightOffUpdate = backLightOffDelay;
  canvas.init();

  delay(1000);
  //Configure the widgets
  //=========================================  
    trend.setSize(220,100);
    trend.setColors(BLACK,GREEN,GRAY1);
    trend.setLimits(70,80,90);
    trend.init();
    trend.setHiLimit(85,RED);
    trend.setLowLimit(75,BLUE);
    trend.enableAutoFit = false;
    
    dial.setSize(50);
    dial.setColors(BLACK,YELLOW,GRAY1);
    dial.setLimits(60,80,100);
    dial.init();
    dial.setHiLimit(85,RED);
    dial.setLowLimit(75,BLUE);

    grid.setSize(80,90);
    grid.setColors(GRAY2,WHITE,WHITE);
    grid.setEventHandler(&gridEventHandler);
    grid.setName(1,"HI");
    grid.setName(2,"SP");
    grid.setName(3,"Low");
    grid.init();
    
    numkey.setSize(120,180);
    numkey.setColors(GRAY1,BLACK,WHITE);
    numkey.init();
    numkey.setNumkeyEventHandler(&numkeyEventHandler);
    numkey.setDebounce(100);
    
  // Add widgets to canvas
  // (Use layout template for coordinates)
  //=========================================
  canvas.add(&trend,10,10);
  canvas.add(&dial,75,180);
  canvas.add(&grid,200,130);
  //numkey not added here (it is a pop-up)
  
  tempF = getTempF(TMP_PIN);
  dial.setCV((int) tempF);
  trend.addValue((uint8_t) tempF);
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

  //Modified scan to return a boolean indicating if a valid
  //touch event happened
  //Now the backlight will turn off after a time delay until a touch event
  //is detected.
  if(canvas.scan()){
    digitalWrite(BACKLIGHT,HIGH);
    backLightOffUpdate=backLightOffDelay;
    //Serial.print("Touched ");Serial.println(millis());
  }else{
    if(backLightOffUpdate-- <= 0) digitalWrite(BACKLIGHT,LOW);
    //Serial.println(backLightOffUpdate);
    delay(200);  
  }
  
}


// Event Handler Functions for buttons
//==========================================
void gridEventHandler(Buttongrid* grid, unsigned char num){
  
  dial.visible = false; // do not redraw dial while numkey is shown
  trend.visible = false;
  
  //Serial.print("Button grid number pressed: ");
  //Serial.println(num);
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

  return;
}

void numkeyEventHandler(Numkey* nk){
	if(nk->getTextSize()>0){
    switch(grid.lastPressed){
      case 1:
        dial.scaleMax = nk->getNum();
        trend.scaleMax = nk->getNum();
        //trend.hiLimit = trend.scaleMax;
        break;
      case 2:
        dial.setpoint = nk->getNum();
        trend.setpoint = nk->getNum();
        break;
      case 3:
        dial.scaleMin = nk->getNum();
        trend.scaleMin = nk->getNum();
        //trend.lowLimit = trend.scaleMin;
        break;
      default:
        break;
    }
  }

  dial.visible = true; // allow dial to redraw again
  trend.hide();  
  trend.visible = true;
  
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
