// Required includes
#include <SPI.h>
#include <StandardCplusplus.h>
#include <vector>

// GUI includes
#include <Gauge.h>
#include <Button.h>
#include <Display.h>

// Create the objects
//==========================================
Canvas canvas = Canvas();
Gauge gauge = Gauge();//Gauge(40,100,0,50,100);
Display valueDisplay = Display();//Display(80,40,BLACK,GREEN,WHITE);
Button btnInc = Button();//Button(50,50,GRAY1,BLACK,WHITE);
Button btnDec = Button();//Button(50,50,GRAY1,BLACK,WHITE);

/*
 *  Setup function
 */
void setup() {
  Serial.begin(9600);
  
  //Configure the objects
  //=========================================
    canvas.init(TFT_LANDSCAPE);
  
    //Prepare widgets
    btnInc.setSize(50,50);
    btnInc.setColors(GRAY1,BLACK,WHITE);
    btnInc.setText("+");
    btnInc.setEventHandler(&incEventHandler);
    btnInc.init();
    btnInc.setDebounce(200);
  
    btnDec.setSize(50,50);
    btnDec.setColors(GRAY1,BLACK,WHITE);
    btnDec.setText("-");
    btnDec.setEventHandler(&decEventHandler);
    btnDec.init();
    btnDec.setDebounce(200);
    
    gauge.setSize(40,100);
    gauge.setColors(BLACK,YELLOW,WHITE);
    gauge.setLimits(0,65,100);
    gauge.init();
    gauge.setHiLimit(85,GREEN);
    gauge.setLowLimit(45,RED);

    valueDisplay.setSize(80,40);
    valueDisplay.setColors(BLACK,YELLOW,WHITE);    
    valueDisplay.setNum(gauge.getCV());
    valueDisplay.init();

    //Add them to the canvas
    canvas.add(&btnInc,120,0);
    canvas.add(&btnDec,120,100);
    canvas.add(&valueDisplay,0,0);
    canvas.add(&gauge,0,50); 
    canvas.setDebounce(200);
}


/*
 *  Main program
 *  Must call canvas scan to check for touch events
 */
void loop() {
  //Updates the canvas and monitor for touch events
  canvas.scan();
}


/*
 *  Auxiliary Functions
 */

// Event Handler Functions for buttons
//==========================================
void incEventHandler(Button* btn){
  gauge.setCV(gauge.getCV()+5);
  valueDisplay.fgColor = gauge.fgColor;
  if(gauge.getCV() >= gauge.hiLimit) valueDisplay.fgColor = gauge.hiLimitColor;
  if(gauge.getCV() <= gauge.lowLimit) valueDisplay.fgColor = gauge.lowLimitColor;
  valueDisplay.setNum(gauge.getCV());
  valueDisplay.show();
}

void decEventHandler(Button* btn){
  if(gauge.getCV()>=5)  gauge.setCV(gauge.getCV()-5);
  valueDisplay.fgColor = gauge.fgColor;
  if(gauge.getCV() >= gauge.hiLimit) valueDisplay.fgColor = gauge.hiLimitColor;
  if(gauge.getCV() <= gauge.lowLimit) valueDisplay.fgColor = gauge.lowLimitColor;
  valueDisplay.setNum(gauge.getCV());
  valueDisplay.show();
}
