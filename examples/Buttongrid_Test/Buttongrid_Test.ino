#include <Buttongrid.h>

Canvas canvas = Canvas(TFT_LANDSCAPE,BLACK);
Buttongrid btnGrid = Buttongrid(4,5,BUTTONGRID_MAINTAINED|BUTTONGRID_HIGHLIGHT|BUTTONGRID_USELABELS);

void setup() {
  Serial.begin(9600);
  canvas.init();

  btnGrid.setLabel(1,"One");
  btnGrid.setLabel(2,"Two");
  btnGrid.setLabel(3,"Three");
  btnGrid.setColors(GRAY1,BLACK,GRAY2);
  btnGrid.setSize(300,220);
  btnGrid.setEventHandler(&btnGridEventHandler);
  canvas.add(&btnGrid,10,10);
}

void loop() {
  canvas.scan(); 
}

void btnGridEventHandler(Buttongrid* btn, int id){
  return;
}

