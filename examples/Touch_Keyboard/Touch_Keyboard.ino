#include <Canvas.h>
#include <Screen.h>
#include <Button.h>
#include <Buttongrid.h>
#include <Display.h>

Canvas canvas = Canvas(TFT_LANDSCAPE,BLACK);//,TOUCHTYPE_ADAFRUIT_CAPACITIVE);
Screen homeScreen = Screen(&canvas);
Screen pwdScreen = Screen(&canvas);
Buttongrid keyboard = Buttongrid(4,10,BUTTONGRID_USELABELS);
Display dsp = Display(304,40,BLACK,GREEN,GRAY2,24);
Button btnPassword = Button(80,40,GRAY2,WHITE,GRAY1);

int keymap = 0;
char keymap1[40][2] = {
  "1","2","3","4","5","6","7","8","9","0",
  "Q","W","E","R","T","Y","U","I","O","P",
  "A","S","D","F","G","H","J","K","L",{190,0},
  {171,0},"Z","X","C","V","B","N","M"," ",{216,0}
};
char keymap2[40][2] = {
  "!","@","#","$","%","^","&","*","(",")",
  "+","-","/","=","_","<",">","[","]","\\",
  ".",",",":",";",{168,0},"?",{173,0},"'","\"",{190,0},
  {171,0}," ","~","`","{","}"," "," "," ",{216,0}
};
char* password = "15SSPKATM";

void setup() {
  Serial.begin(115200);
  canvas.init();

  dsp.setText("");
  keyboard.setDebounce(100);
  keyboard.borderWidth = 1;
  for(char i=0;i<40;i++){
    Serial.print(keymap1[i]);
    //char key[8] = {keymap1[i],0,0,0,0,0,0,0};
    keyboard.setLabel(i+1,keymap1[i]);
  }
  Serial.println();
  keyboard.setSize(304,170);
  keyboard.setColors(GRAY1,BLACK,WHITE);
  keyboard.setEventHandler(&keyboardEventHandler);

  pwdScreen.x = 0;
  pwdScreen.y = 0;
  pwdScreen.w = 320;
  pwdScreen.h = 240;
  pwdScreen.add(&dsp,5,10);
  pwdScreen.add(&keyboard,5,50);

  btnPassword.setText("Password");
  btnPassword.setEventHandler(&btnPasswordEventHandler);
  homeScreen.x = 0;
  homeScreen.y = 0;
  homeScreen.w = 320;
  homeScreen.h = 240;
  homeScreen.add(&btnPassword,120,100);
  canvas.setScreen(&pwdScreen);
}

void loop() {
  canvas.scan();
}

void btnPasswordEventHandler(Button* btn){
  canvas.setScreen(&pwdScreen);
}

bool matchPassword(){
  unsigned char pwdSize = Widget::getTextLength(password);
  if(pwdSize != dsp.contents->getTextSize()){
    return false;
  }
  for(int i=0; i<pwdSize; i++){
    char baseChar = dsp.contents->text[i];
    char lowerChar,upperChar;
    Serial.print(baseChar);
    //Check if baseChar is a CAPS character or a LOWER character
    if(baseChar >= 65 && baseChar < 91 || baseChar >= 97 && baseChar < 123){

      //Check if it is CAPS, and get a lower case copy in lowerChar
      if(baseChar >= 65 && baseChar < 91){
        lowerChar = baseChar + 32; //Convert to lowercase
        if(lowerChar != password[i] && baseChar != password[i]) {
          return false;
        }
      }

      //Check if it is lower case and convert to upper case
      if(baseChar >= 97 && baseChar < 123){
        upperChar = baseChar - 32; //Convert to lowercase
        if(upperChar != password[i] && baseChar != password[i]) {
          return false;
        }
      }
    }
    Serial.println();
  }
  return true;
}

void keyboardEventHandler(Buttongrid* kbd, unsigned char key){
  // Backspace key
  if(key == 30){
    dsp.deleteChar();
    dsp.update();
    return;
  }
  // Enter key
  if(key == 40){
    Serial.println(dsp.contents->text);
    if(matchPassword()) canvas.setScreen(&homeScreen);
    dsp.setText("");
    dsp.update();
    return;
  }
  // Alt key
  if(key == 31){
    //Choose overlay for buttons
    if(keymap == 0){
      keymap = 1; 
    }else{
      keymap = 0;
    } 
    for(int i=0;i<40;i++){
      if(keymap){
        keyboard.setLabel(i+1,keymap2[i]);
      }else{
        keyboard.setLabel(i+1,keymap1[i]);
      }
      //Serial.print(keyboard.labels[i]);
    }
    //Serial.println();
    keyboard.show();
    return;
  }
  if(kbd->labels[key]){    
    dsp.append(kbd->labels[key]);
    dsp.update();
  }
  return;
}

