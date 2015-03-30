#include <SPI.h>
#include <Numkey.h>
#include <Dial.h>

Canvas canvas = Canvas();
Dial dial = Dial();
Numkey numkey = Numkey();
Button btnShow = Button(80,40,BLACK,GREEN,GRAY2);
Button btnInc = Button(40,40,GRAY1,BLACK,WHITE);
Button btnDec = Button(40,40,GRAY1,BLACK,WHITE);

const int pingPin = 22; //D22 in Mega
unsigned long pingDelay = 0;
long pingLastVal = 0;

void setup() {
  Serial.begin(9600);
  canvas.init(TFT_LANDSCAPE);
  
  dial.setSize(50);
  dial.setColors(BLACK,YELLOW,WHITE);
  dial.setLimits(0,100,300);
  dial.init();
  dial.setHiLimit(200,RED);
  dial.setLowLimit(30,GREEN);
  
  btnShow.setText("Show");
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
  
  canvas.add(&btnShow,0,0);
  canvas.add(&btnInc,250,10);
  canvas.add(&btnDec,250,50);
  canvas.add(&dial,numkey.w+20+dial.radius,dial.radius);
}

void loop() {
  canvas.scan();
  if(millis()>pingDelay+500){
    long val = getDistance(pingPin);
    if(val != pingLastVal){
      dial.setCV(val);
      pingLastVal = val;
    }
    pingDelay = millis();
  }
}

// Event Handlers
//===================================
void numkeyEventHandler(Numkey* nk){
  if(*nk->getText()){
    dial.setCV(nk->getNum());
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
  int val = dial.getCV();
  if(val<dial.scaleMax){
    dial.setCV(constrain(val+inc,dial.scaleMin,dial.scaleMax));
    btnShow.setNum(dial.getCV());
  }
}

void decValueEventHandler(Button* btn){
  int dec = 5;
  int val = dial.getCV();
  if(val>dial.scaleMin){
    dial.setCV(constrain(val-dec,dial.scaleMin,dial.scaleMax));
    btnShow.setNum(dial.getCV());
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
    
  return cm;//pingLastVal;
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

