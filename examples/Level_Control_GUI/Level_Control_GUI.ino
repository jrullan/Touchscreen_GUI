#include <SPI.h>
#include <Numkey.h>
#include <Dial.h>

Canvas canvas = Canvas();
Dial dialCm = Dial();
Dial dialIn = Dial();
Numkey numkey = Numkey();
Button btnShow = Button(20,BLACK,GREEN,GRAY2);
Button btnInc = Button(20,GRAY1,BLACK,WHITE);
Button btnDec = Button(20,GRAY1,BLACK,WHITE);

const int pingPin = 22; //D22 in Mega
const int relayPin = 24;
unsigned long pingDelay = 0;
long pingLastVal = 0;

void setup() {
  pinMode(relayPin,OUTPUT);
  
  Serial.begin(9600);
  canvas.init(TFT_LANDSCAPE);
  
  dialCm.setSize(50);
  dialCm.setColors(BLACK,YELLOW,WHITE);
  dialCm.setLimits(0,100,300);
  dialCm.init();
  dialCm.setHiLimit(200,RED);
  dialCm.setLowLimit(30,GREEN);

  dialIn.setSize(50);
  dialIn.setColors(BLACK,BLUE,WHITE);
  dialIn.setLimits(0,48,84);
  dialIn.init();
  dialIn.setHiLimit(72,RED);
  dialIn.setLowLimit(12,GREEN);

  
  //btnShow.setText("Show");
  btnShow.init();
  btnShow.setEventHandler(&showEventHandler);

  btnInc.setText("+");
  btnInc.setEventHandler(&incValueEventHandler);
  btnInc.init();
  btnInc.setDebounce(200);

  btnDec.setText("-");
  btnDec.setEventHandler(&decValueEventHandler);
  btnDec.init();
  btnDec.setDebounce(200);

    
  numkey.setSize(120,160);
  numkey.setColors(GRAY1,BLACK,WHITE);
  //numkey.setNum(100);
  numkey.init();
  numkey.setNumkeyEventHandler(&numkeyEventHandler);
  
  canvas.add(&btnShow,0,10);
  canvas.add(&btnInc,90,10);
  canvas.add(&btnDec,90,50);
  canvas.add(&dialCm,numkey.w+30+dialCm.radius,dialCm.radius);
  canvas.add(&dialIn,numkey.w+30+dialCm.radius,dialCm.radius*3+10);
}

void loop() {
  canvas.scan();
  if(millis()>pingDelay+500){
    long val = getDistance(pingPin);
    int cm = microsecondsToCentimeters(val);
    int inches = microsecondsToInches(val);
    //Only update dials if value changes
    if(cm != pingLastVal){
      dialCm.setCV(cm);
      dialIn.setCV(inches);
      
      //Turn "on" the button if value crosses limit
      if(cm > 42 ){//&& pingLastVal <= 42){
        btnShow.drawBackground(GREEN);
        digitalWrite(relayPin,HIGH);
      }
      
      //Turn "off" the button if value is restored
      if(cm <= 35){// && pingLastVal > 35){
          btnShow.show();
          digitalWrite(relayPin,LOW);
      }
      
      pingLastVal = cm;
    }
    pingDelay = millis();
  }
}

// Event Handlers
//===================================
void numkeyEventHandler(Numkey* nk){
  if(*nk->getText()){
    dialCm.setCV(nk->getNum());
    btnShow.setNum(nk->getNum());
  }
  nk->clear();
  canvas.pop(); //Remove the keypad from the canvas
}

void showEventHandler(Button* btn){
  canvas.add(&numkey,0,0);
}

void incValueEventHandler(Button* btn){
  int inc = 5;
  int val = dialCm.getCV();
  if(val<dialCm.scaleMax){
    dialCm.setCV(constrain(val+inc,dialCm.scaleMin,dialCm.scaleMax));
    btnShow.setNum(dialCm.getCV());
  }
}

void decValueEventHandler(Button* btn){
  int dec = 5;
  int val = dialCm.getCV();
  if(val>dialCm.scaleMin){
    dialCm.setCV(constrain(val-dec,dialCm.scaleMin,dialCm.scaleMax));
    btnShow.setNum(dialCm.getCV());
  }
}


// Ping related functions
//============================================
int getDistance(int pin){
  long duration, inches, cm;
    // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    pinMode(pingPin, OUTPUT);
    digitalWrite(pingPin, LOW);
    delayMicroseconds(2);
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(pingPin, LOW);
  
    // The same pin is used to read the signal from the PING))): a HIGH
    // pulse whose duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(pingPin, INPUT);
    duration = pulseIn(pingPin, HIGH);
  
    // convert the time into a distance
    inches = microsecondsToInches(duration);
    cm = microsecondsToCentimeters(duration);
    
    //pingLastVal = cm;
    
  return duration;//pingLastVal;
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

