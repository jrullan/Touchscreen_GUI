#include "Numkey.h"
#include <stdio.h>

Numkey::Numkey(){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
	this->init();
}

Numkey::Numkey(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
	this->init();

	this->setSize(width,height);
	this->setColors(backgroundColor,textColor,borderColor);
}

Numkey::~Numkey(){
	if(text) free(text);
}

void Numkey::init(){
	Button::init();
	type = 0x32;
	borderWidth = 2;
	charPos = 0;
	setSize(120,180);
	debounceTime = 100;
	autoremove = true;
}

void Numkey::drawFrame(){
    if (!canDraw()) return;
  int xPos,yPos,width,height;
  int btnWidth,btnHeight;
  
  xPos = x;	
  yPos = y;
  width = w;
  height = h;
  btnWidth = w / 3;
  btnHeight = h / 5;
  
  //--display background
  myCanvas->tft->fillRect(xPos,yPos,btnWidth*2,btnHeight,BLACK);
  //--backspace background
  myCanvas->tft->fillRect(xPos+btnWidth*2,yPos,btnWidth,btnHeight,bgColor);
  //--nums background rectangle
  myCanvas->tft->fillRect(xPos,yPos+btnHeight,width,height-btnHeight,bgColor);
  
	//-- outer border
  for(byte i=borderWidth; i!=0;i--){
    myCanvas->tft->drawRect(xPos,yPos,width,height,borderColor);
    xPos++;
    yPos++;
    width--;
    height--;
  }
  
  //-- horizontal lines
  for(byte j=1;j<5;j++)
  {
  	for(byte i=0; i<borderWidth;i++){
    	myCanvas->tft->drawHorizontalLine(x,y+btnHeight*j-1+i,width+borderWidth,borderColor);
  	}
  }
  
  //-- vertical lines
  for(byte j=1;j<3;j++)
  {
  	for(byte i=0; i<borderWidth;i++){
    	myCanvas->tft->drawVerticalLine(x+btnWidth*j-1+i,y+btnHeight,height-btnHeight+borderWidth,borderColor);
  	}
  }

	//-- draw numbers  
  xPos = btnWidth/2 - FONT_X;
  yPos = btnHeight/2 - FONT_Y;
  byte xF=0;
  byte yF=1;
  myCanvas->tft->drawString("<-",x+xPos+2*btnWidth-6,y+yPos,fontSize,fgColor); 
  for(byte number=1; number<10; number++)
  {
  	myCanvas->tft->drawNumber(number,x+xPos+(xF++)*btnWidth,y+yPos+(yF*btnHeight),fontSize,fgColor);
  	if(number == 3 || number == 6 || number == 9){
  		yF++;
  		xF = 0;
  	}
  }
  myCanvas->tft->drawString(".",x+xPos+(xF++)*btnWidth,y+yPos+(yF*btnHeight),fontSize,fgColor);  
  myCanvas->tft->drawNumber(0,x+xPos+(xF++)*btnWidth,y+yPos+(yF*btnHeight),fontSize,fgColor);
  myCanvas->tft->drawString("=",x+xPos+(xF++)*btnWidth,y+yPos+(yF*btnHeight),fontSize,fgColor);
}

void Numkey::append(char* c) {
    if (!text || !c) return;
    size_t used = getTextSize(), count = 0;
    while (count < DISPLAY_SIZE - used && c[count]) ++count;
    memmove(text + used, c, count);
    text[used + count] = 0;
    update();
}

void Numkey::appendNum(unsigned char c){
    if (!text) return;
	//Serial.print("Append num ");Serial.println((byte)c);

	if(c > 9){ 
		//Serial.print("Not a number");Serial.write(c);
		return; //Exit if c is not a number 0 - 9
	}
	byte txtSize = getTextLength(text);//getTextSize();
	byte space = DISPLAY_SIZE - txtSize;
	//Check that there space available to append
	if(txtSize < DISPLAY_SIZE){
		text[txtSize] = c + 48;
		text[txtSize + 1] = 0;
		update();
	}
}

void Numkey::fitToText(){
  if(text && *text){
    char* chars = text;
    char size = 0;
    while(*chars){
      *chars++;
      size++;
    }
    w = size * FONT_SPACE * borderWidth + FONT_SPACE;
    h = FONT_Y * borderWidth + FONT_Y;
    //drawString(text,x+5,y+5,2,textColor);
  }
}

long Numkey::getNum() {
    return text ? strtol(text, NULL, 10) : 0;
}

char* Numkey::getText(){
  return text;
}

byte Numkey::getTextLength(char* c) {
    byte length = 0;
    if (c) while (length < DISPLAY_SIZE && c[length]) ++length;
    return length;
}

byte Numkey::getTextSize(){
  return getTextLength(text);
}

void Numkey::setNum(int num) {
    if (!text) return;
    snprintf(text, DISPLAY_SIZE + 1, "%d", num);
}

void Numkey::setEventHandler(void (*functionPointer)(Numkey *)){
	eventHandler = functionPointer;
}

/*
void Numkey::setText(char* _text){
	
	for(int i=0; i<DISPLAY_SIZE-1;i++){
		text[i] = _text[i];
	}
	update();
  //Serial.print("Set text to ");Serial.println(text);
}
*/

void Numkey::deleteChar() {
    if (!text) return;
    byte used = getTextSize();
    if (used) text[used - 1] = 0;
    if (canDraw()) myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,(w/3*2)-borderWidth,h/5-borderWidth*2,BLACK);
    update();
}

void Numkey::clear() {
    if (text) memset(text, 0, DISPLAY_SIZE + 1);
    if (canDraw()) myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,(w/3*2)-borderWidth,h/5-borderWidth*2,BLACK);
}

//Overriden virtual methods

bool Numkey::checkTouch(Point* p){
    if (!p) return false;
	int boundX1, boundX2, boundY1, boundY2;
	int btnWidth = w/3;
	int btnHeight = h/5;
	bool pressed = false;
	if(lastMillis + debounceTime < millis()){ 
		if((p->x > x+borderWidth) && (p->x < x+w-borderWidth) && (p->y > y+borderWidth) && (p->y < y+h-borderWidth)){
			//Serial.println("Numkey event within bounds");
			//backspace coordinates
			boundX1 = x+btnWidth*2;
			boundX2 = x+w;
			boundY1 = y+borderWidth;
			boundY2 = y+btnHeight;
			if((p->x > boundX1) && (p->x < boundX2) && (p->y > boundY1) && (p->y < boundY2)){
				//Serial.println("Backspace pressed");
				deleteChar();
				pressed = true;
			}
			//num coordinates
			for(int r = 1;(r <= 4)&&(!pressed); r++)
			{
				boundY1 = y+(btnHeight)*r+borderWidth;
				boundY2 = y+(btnHeight)*(r+1)-borderWidth;
				for(int c = 1;(c <= 3)&&(!pressed);c++)
				{
					boundX1 = x + (btnWidth)*(c-1) + borderWidth;
					boundX2 = x + (btnWidth)*c - borderWidth;
					int num = 3*(r - 1) + c;

					if((p->x > boundX1) && (p->x < boundX2) && (p->y > boundY1) && (p->y < boundY2)){
						//Serial.print(num); Serial.println();
						//Serial.println("A number has been pressed");
						
						if(num==11){
							//Serial.println("0 pressed");
							num = 0;
						}
						
						if(num==10){
							//Serial.println(". pressed");
							char decimalPoint[] = ".";
							append(decimalPoint);
						}else if(num==12){
							//Serial.println("= pressed");
							entry = getNum();
							if(autoremove && myCanvas){
								if(!myCanvas->widgets.isEmpty() && myCanvas->widgets.peek() == this){ // check if top widget is this numkey
									myCanvas->pop();				  // Remove this numkey from the canvas...
									//this->hide();
								}
							}
							if (eventHandler) eventHandler(this);		// <<<------ Event handler called when = signed is pressed
						}else{
							appendNum(char(num));//setNum(num);
						}
						
						
						pressed = true;
					}
				}// -- columns for loop
			}// --rows for loop
		}// -- if touch within widget area
		touched = !touched;
		lastMillis = millis();		
	}// -- debounce
	return false;
}

void Numkey::show(){
    visible = true;
	drawFrame();
	update();
	if(!visible) visible = true;
}

void Numkey::show(Button* btn){
	targetButton = btn;
	show();
}

void Numkey::targetEventHandler(){
	if (targetButton && targetButton->eventHandler) targetButton->eventHandler(targetButton);
}

void Numkey::reset(){
	entry = 0;
}

/* New method to make it easier to use,
 * automatically redraw the canvas when hidden.
 */
void Numkey::hide() {
    const bool drawable = canDraw();
    visible = false;
    if (!drawable) return;
    myCanvas->tft->fillRect(x,y,w,h,myCanvas->bgColor);
    myCanvas->redraw();
}

void Numkey::update(){
    if (!canDraw()) return;
  int btnWidth = w / 3;
  int btnHeight = h / 5;
  
  if(text && *text){
    char* chars = text;
		
		//Count characters to center on the button - Nice trick from the myCanvas->tft2 library
    int size = 0;
    while(*chars){
      *chars++;
      size++;
    }
    //Calculate centered position of the text
    int stringX = getCenterTextX(x,btnWidth*2,size);
    int stringY = getCenterTextY(y,btnHeight);

		// clear area
		myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,(btnWidth*2)-borderWidth,btnHeight-borderWidth*2,BLACK);  

    // draw remaining characters
		myCanvas->tft->drawString(text,stringX,stringY,borderWidth,GREEN);
  }
}
