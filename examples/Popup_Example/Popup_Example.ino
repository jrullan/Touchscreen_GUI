/**************************************************************
 *
 *  Popup Example
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
#include <SPI.h>
#include <Popup.h>

#if defined(ESP32)
  Guitft Tft = Guitft(5,26);
#else
  Guitft Tft = Guitft(5,6);
#endif

Canvas canvas = Canvas(TFT_LANDSCAPE,BLACK);
Popup popup = Popup();

void setup() {
  Serial.begin(9600);
  canvas.init();
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH);

  popup.setSize(220,100);
  popup.setColors(GRAY1,BLACK,WHITE);
  popup.setText("Do you like it?");
  popup.setEventHandler(&popupEventHandler);
  popup.popup_type = POPUP_YES_NO;
  popup.init();
  popup.visible = true;
  //popup.setDebounce(100);
  
  canvas.add(&popup,50,70);
}

void loop() {
  canvas.scan();
}

void popupEventHandler(Popup* pup, unsigned char val){
  Serial.print("Button ");Serial.print(val);
  Serial.println(" pressed");
  
  if(pup->popup_type == POPUP_YES_NO){
    
    if(val==1){
      pup->setText("Nothing to do");
    }else{
      pup->setText("Should've said Yes");
    } 
    pup->popup_type = POPUP_OK;
    
  }else if(pup->popup_type == POPUP_OK){
    
    pup->setText("Do you like it?");
    pup->popup_type = POPUP_YES_NO;
    
  }
  
  pup->visible = true;
  canvas.redraw();
  showPopUpType();
}


void showPopUpType(){
  Tft.fillRectangle(0,0,200,40,BLACK);
  String txt = "Pop-Up Type: ";
  txt += popup.popup_type;
  char buf[16];
  txt.toCharArray(buf,15);
  Tft.drawString(buf,0,0,1,YELLOW);
}
