#include <neotimer.h>

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

#define TMP_PIN 19
#define tempUpdate 100
#define trendUpdate 100
#define backLightOffDelay 100

Canvas canvas = Canvas(TFT_PORTRAIT,BLACK);
Dial dial;
Numkey numkey;
Buttongrid grid = Buttongrid(3,1);
Trend trend;
Gauge gauge = Gauge(40,100,30,80,100);

float tempF;
long lastUpdate,lastTrendUpdate, backLightOffUpdate;
Neotimer updateTrend = Neotimer(1000);
Neotimer updateGauge = Neotimer(1000);
Neotimer updateDisplay = Neotimer(1000);
Neotimer displayOffTimer = Neotimer(60000);

int degree = 0;

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
    dial.setLimits(30,80,100);
    dial.init();
    dial.setHiLimit(85,RED);
    dial.setLowLimit(50,BLUE);

    grid.setSize(80,90);
    grid.setColors(GRAY2,WHITE,WHITE);
    grid.setEventHandler(&gridEventHandler);
    grid.setName(1,"HI");
    grid.setName(2,"SP");
    grid.setName(3,"Low");
    grid.init();

    gauge.setColors(BLACK,GREEN,GRAY2);
    gauge.setLowLimit(50,BLUE);
    gauge.setSP(80,GREEN);
    gauge.setHiLimit(85,RED);
    gauge.init();
      
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
  //canvas.add(&grid,145,130);
  canvas.add(&gauge,145,130);
  //numkey not added here (it is a pop-up)
  
  tempF = getTempF(TMP_PIN);
  dial.setCV((int) tempF);
  trend.addValue((uint8_t) tempF);
  
  displayOffTimer.start();
}


void loop() {
  int lastTemp = (int)tempF;
  tempF = getTempF(TMP_PIN);
  //tempF = random(70,90);
  if(updateTrend.repeat(10)){
    int trendPoint = map(isin(degree),-127,127,70,90);
    degree = (degree>=360) ? 0 : degree+8; 
    trend.addValue((uint8_t)trendPoint);
  }

  if(updateDisplay.repeat(1000)){
    //trend.addValue((uint8_t) tempF);
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
  }

  if(updateGauge.repeat(random(1000,30000))){
    //gauge.setCV(random(73,90));
    gauge.setCV((int)tempF);
  }

  if(displayOffTimer.done()){
    digitalWrite(BACKLIGHT,LOW);
  }
  
  canvas.scan();
  
  if(canvas.touchedPoint.x > 50 && canvas.touchedPoint.x < 200 && canvas.touchedPoint.y > 100 && canvas.touchedPoint.y < 200){
    displayOffTimer.stop();
    displayOffTimer.reset();
    displayOffTimer.start();
    digitalWrite(BACKLIGHT,HIGH);
  }

/*

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
*/
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
  canvas.add(&numkey, 80,5);

  return;
}

void numkeyEventHandler(Numkey* nk){
	if(nk->getTextSize()>0){
    switch(grid.lastPressed){
      case 1:
        dial.hiLimit = nk->getNum();
        trend.hiLimit = nk->getNum();
        //dial.scaleMax = nk->getNum();
        //trend.scaleMax = nk->getNum();
        //trend.hiLimit = trend.scaleMax;
        break;
      case 2:
        dial.setpoint = nk->getNum();
        trend.setpoint = nk->getNum();
        break;
      case 3:
        dial.lowLimit = nk->getNum();
        trend.lowLimit = nk->getNum();
        //dial.scaleMin = nk->getNum();
        //trend.scaleMin = nk->getNum();
        //trend.lowLimit = trend.scaleMin;
        break;
      default:
        break;
    }
    canvas.redraw();
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
  int a = analogRead(PIN)>>2;
  float resistance;
  int B = 3975; //thermistor constant

  resistance = (float) (1023-a)*10000/a;
  //resistance = (float) (4095-a)*10000/a;
  
  return 1/(log(resistance/10000)/B + 1/298.15) - 273.15;
}

float getTempF(int PIN){
  return getTempC(PIN)*9.0/5.0+32.0;
}


uint8_t isinTable8[] = {
  0, 4, 9, 13, 18, 22, 27, 31, 35, 40, 44,
  49, 53, 57, 62, 66, 70, 75, 79, 83, 87,
  91, 96, 100, 104, 108, 112, 116, 120, 124, 128,

  131, 135, 139, 143, 146, 150, 153, 157, 160, 164,
  167, 171, 174, 177, 180, 183, 186, 190, 192, 195,
  198, 201, 204, 206, 209, 211, 214, 216, 219, 221,

  223, 225, 227, 229, 231, 233, 235, 236, 238, 240,
  241, 243, 244, 245, 246, 247, 248, 249, 250, 251,
  252, 253, 253, 254, 254, 254, 255, 255, 255, 255,
  };

int isin(long x)
{
  boolean pos = true;  // positive - keeps an eye on the sign.
  if (x < 0)
  {
    x = -x;
    pos = !pos; 
  } 
  if (x >= 360) x %= 360;   
  if (x > 180)
  {
    x -= 180;
    pos = !pos;
  }
  if (x > 90) x = 180 - x;
   if (pos) return isinTable8[x]/2 ;
  return -isinTable8[x]/2 ;
}
