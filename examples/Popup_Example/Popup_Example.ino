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
  popup.setText("Do you like it?","Yes","No");
  popup.setEventHandler(&popupEventHandler);
  popup.type = 1;
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
  if(pup->type){
    pup->type = 0;
    if(val==1){
      pup->setText("Nothing to do","Ok");
    }else{
      pup->setText("Should've said Yes","Ok");
    }
  }else{
    pup->type = 1;
    pup->setText("Do you like it?","Yes","No");
  }
  pup->visible = true;
  canvas.redraw();
}
