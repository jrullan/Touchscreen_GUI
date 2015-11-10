  /**************************************************************
 *
 *  Touchscreen_GUI_Template
 *
 *  Use this template as a starting point.
 *  The required libraries "includes" and configuration
 *  sample code is provided to help you quickly create
 *  a working sketch.
 *
 *  This library was developed for Seeedstudio
 *  TouchShield v2.0, based on their TFTv2 and SeeedTouchscreen
 *  libraries. This library is intended to provide a minimal
 *  but useful collection of reusable widgets that can be used
 *  to create a Graphical User Interface (GUI) for any Arduino 
 *  project.
 *
 *  Comment out (or delete) lines that are not required for your project.
 *
 *  Author: Jose Rullan
 *	Release Date: 03/28/2015  -- Arduino Day 2015!!!
 *	Release Version: 0.1 beta
 *
 *	License: Whatever only requires to give credit to original 
 *	author (i.e. Jose Rullan)
 *
 *	Credits:
 *	
 *  
 *************************************************************/

// Required includes 
#include <SPI.h>

// TouchscreenGUI includes
#include <Display.h>
#include <Buttongrid.h>

// Create the objects
//==========================================
Canvas canvas = Canvas(); // Memory used: (storage/ram: 1,676/36)  3,372/228
Display rowDisplay = Display(); // Memory used: (storage/ram: 484/37)    12,240/392
Display colDisplay = Display();
Display numberDisplay = Display();
Button btnClear = Button();
Buttongrid buttonGrid = Buttongrid(5); // Memory used: (storage/ram: 2,370/59)  16,080/503

// Global variables
// If you need global variables in your program put them here,
// before the setup() routine.
unsigned char B[5];
unsigned char I[5];
unsigned char N[5];
unsigned char G[5];
unsigned char O[5];

void setup() {
  Serial.begin(9600);
  canvas.init(TFT_LANDSCAPE);

  //Configure the widgets
  //=========================================  
    rowDisplay.setSize(80,25);
    rowDisplay.setColors(BLACK,GREEN,GRAY2);
    rowDisplay.setText("");
    rowDisplay.init();

    colDisplay.setSize(80,25);
    colDisplay.setColors(BLACK,GREEN,GRAY2);
    colDisplay.setText("");
    colDisplay.init();

    numberDisplay.setSize(80,25);
    numberDisplay.setColors(BLACK,GREEN,GRAY2);
    numberDisplay.setText("");
    numberDisplay.init();

    btnClear.setSize(80,40);
    btnClear.setColors(GRAY1,BLACK,WHITE);
    btnClear.setText("Clear");
    btnClear.setEventHandler(&buttonEventHandler);
    btnClear.init();
    btnClear.setDebounce(100);
    
    buttonGrid.setSize(220 ,240);
    buttonGrid.setColors(GRAY1,BLACK,WHITE);
    buttonGrid.init();
    buttonGrid.setEventHandler(&buttonGridEventHandler);
    buttonGrid.setDebounce(100);
    
  // Add widgets to canvas
  // (Use layout template for coordinates)
  //=========================================

  //setBingoLabels();
  canvas.add(&rowDisplay,5,0);
  canvas.add(&colDisplay,5,50);  
  canvas.add(&numberDisplay,5,100);  
  canvas.add(&btnClear,5,150);
  canvas.add(&buttonGrid,100,0);
  
  delay(1000);
}


void loop() {
  /* 
   *  Canvas.scan() does all updating of the widgets contained
   *  in the canvas. These were added in setup(). For example,
   *  if a button is pressed scan() calls the button's event
   *  handler function. These are defined below. For dials, 
   *  gauges and other display widgets, if the value of the
   *  display changes, scan() will update the widget accordingly.
   */
  canvas.scan();
}


// Event Handler Functions for buttons
//==========================================

// This declaration must conform to the declaration of 
// the eventHandler in the Buttongrid class.
void buttonGridEventHandler(Buttongrid* bg, INT8U val){
  byte gridSize = val;

  rowDisplay.setNum(bg->getRow(val));
  colDisplay.setNum(bg->getColumn(val));
  numberDisplay.setNum(val);

  //bg->configure(gridSize,gridSize,bg->font_size);

}

void buttonEventHandler(Button* btn){
  Serial.println("Clear pressed");
  //setBingoLabels();

  buttonGrid.clear();
  rowDisplay.clear();
  colDisplay.clear();
  
  numberDisplay.clear();
}

// Generates elements amount of unique random numbers
// in the range from-to and store them in array
void getRandom(unsigned char elements, unsigned char from, unsigned char to, unsigned char* array){
  for(unsigned char i=elements; i!=0; i--)
  {
    boolean found;
    unsigned char val;
    unsigned char index = elements - i;

    // Generates a random number
    // and checks if it is already in the array. 
    // If it is, do it again until it is unique.
    do
    {
      found=false;
      val = random(from, to+1);
      for(unsigned char j=index; j!=0; j--)
      {
        if(array[index-j] == val){
          found=true;
          break; //<-- early exit
        }
      }
    }while(found);
    
    array[index] = val;
    Serial.print(array[index]);Serial.print(",");
  }
  Serial.println();
}

// Generates bingo labels and store then in 
// displayGrid array
void setBingoLabels(){
  Serial.print("B: ");
  getRandom(5,1,15,B);
  Serial.print("I: ");
  getRandom(5,16,30,I);
  Serial.print("N: ");
  getRandom(5,31,45,N);
  Serial.print("G: ");
  getRandom(5,46,60,G);
  Serial.print("O: ");
  getRandom(5,61,75,O);

  for(int i=0; i<buttonGrid.rows; i++)
  {
     buttonGrid.labels[i][0] = B[i];
     buttonGrid.labels[i][1] = I[i];
     buttonGrid.labels[i][2] = N[i];
     buttonGrid.labels[i][3] = G[i];
     buttonGrid.labels[i][4] = O[i];
  }
}

