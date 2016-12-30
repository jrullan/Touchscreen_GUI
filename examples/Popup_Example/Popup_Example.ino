#include <SPI.h>
#include <Popup.h>

Canvas canvas = Canvas();
Popup popup = Popup();

void setup() {
  Serial.begin(9600);
  canvas.init(TFT_LANDSCAPE);
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH);

  popup.setSize(220,100);
  popup.setColors(GRAY1,BLACK,WHITE);
  popup.setText("Do you like it?");
  popup.setEventHandler(&popupEventHandler);
  popup.type = POPUP_YES_NO;
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

  if(pup->type == POPUP_YES_NO){
    
    if(val==1){
      pup->setText("Nothing to do");
    }else{
      pup->setText("Should've said Yes");
    } 
    pup->type = POPUP_OK;
    
  }else if(pup->type == POPUP_OK){
    
    pup->setText("Do you like it?");
    pup->type = POPUP_YES_NO;
    
  }
  
  pup->visible = true;
  canvas.redraw();
}
