#include "Display.h"

Display::Display(){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
}

Display::Display(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
	x = 0;
	y = 0;
	this->setSize(width,height);
	this->setColors(backgroundColor,textColor,borderColor);
	this->init();
}

Display::~Display(){
	if(text) free(text);
}

void Display::init(){
	type = 0x10;
	borderWidth = 2;
	charPos = 0;
}

void Display::drawFrame(){
  int xPos = x;	
  int width = w;
  byte yPos = y;
  byte height = h;
  for(byte i=borderWidth; i!=0;i--){
    Tft.drawRectangle(xPos++,yPos++,width--,height--,borderColor);
    width--;
    height--;
  }
}

void Display::append(char* c){
	byte cSize = getTextLength(c);
	byte txtSize = getTextLength(text);//getTextSize();
	byte space = DISPLAY_SIZE - txtSize;
	
	//Check that there space available to append
	if(txtSize < DISPLAY_SIZE){
		//Serial.print("Space available: ");Serial.println(space);
		for(int i=0; i<space; i++)
		{
			text[i+txtSize] = c[i];
		}
	}
	update();
}

unsigned char Display::getTextLength(char* c){
	char size = 0;
	if(*c){
    char* chars = c;
    while(*chars){
      *chars++;
      size++;
    }
  }
  return size;
}

void Display::fitToText(){
  if(*text){
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

char* Display::getText(){
  return text;
}

long Display::getNum(){
	char size = getTextSize();
	long result = 0;
	for(int i = 0; i<size; i++){
		if(text[i] == '.') break;  // Only process integer side
		result = result * 10 + text[i]-'0';
	}
	return result;
}

void Display::setNum(int num){
	clear();
	char numChar[DISPLAY_SIZE];
	char chars = 0;
	
	// Extract characters representing the powers of ten
	while(num >= 10)
	{
		numChar[chars++] = num%10;
		num /= 10;
		//Serial.print("num ");Serial.println(num);
	}
	
	numChar[chars++] = num;
	
	for(int j = 0; j < chars; j++)//DISPLAY_SIZE; j++)
	{
		text[chars-1-j] = '0'+numChar[j];
		//Serial.print("text[i] ");Serial.println(text[j]);
	}
	
	text[chars]=0;
	update();
}

void Display::setText(char* _text){
	
	for(int i=0; i<DISPLAY_SIZE-1;i++){
		text[i] = _text[i];
	}
	update();
  //Serial.print("Set text to ");Serial.println(text);
}

void Display::deleteChar(){
	byte textSize = getTextSize();
	//Serial.print("Size is ");Serial.println(textSize);
	if(textSize){
		for(int i = textSize-1; i >= 0; i--)
		{
			if(text[i] != 0){
				text[i] = 0;
				break;
			}
		}
	}
	update();
}

unsigned char Display::getTextSize(){
  return getTextLength(text);
}

void Display::clear(){
	byte textSize = getTextSize();
	if(textSize){
		for(int i = textSize-1; i >= 0; i--)
		{
				text[i] = 0;
		}
	}
}

//Overriden virtual methods

bool Display::checkTouch(Point* p){
	return true;
}

bool Display::isButton(){
	return false;
}

void Display::show(){
	drawFrame();
  update();
}

void Display::update(){
  Tft.fillRectangle(x+borderWidth, y+borderWidth, w-(2*borderWidth),h-(2*borderWidth),bgColor);

  //Count characters to center on the button - Nice trick from the Tft2 library
  if(*text){
    char* chars = text;
    char size = 0;
    while(*chars){
      *chars++;
      size++;
    }
    //Calculate centered position of the text
    int stringX = x+(w-size*6*borderWidth)/2;
    //int stringX = x+(w-size*FONT_SPACE*borderWidth)/2;
    int stringY = y+(h-8*borderWidth)/2;
    //int stringY = y+(h-FONT_Y*borderWidth)/2;
    Tft.drawString(text,stringX,stringY,borderWidth,fgColor);
  }	
}
