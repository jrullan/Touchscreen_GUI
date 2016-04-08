#include "Indicator.h"

Indicator::Indicator(){
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
}

Indicator::~Indicator(){
	
}

void Indicator::clear(){
	for(int i = BUF_SIZE-1; i >= 0; i--)
	{
			buf[i] = 0;
	}
}

unsigned int Indicator::getCV(){
	return currentValue;
}

void Indicator::init(){
	x=0;
	y=0;
	borderWidth=3;
}

void Indicator::setCV(int cv){
	previousValue = currentValue;
	currentValue = constrain(cv,scaleMin,scaleMax);
	update();
}

void Indicator::setHiLimit(unsigned int h, int color){
	this->hiLimit = h;
	this->hiLimitColor = color;
}

void Indicator::setLimits(unsigned int min, unsigned int sp, unsigned int max){
	this->hiLimit = max;
	this->lowLimit = min;
	this->currentValue = sp;
	
	this->scaleMax = max;
	this->scaleMin = min;
	this->setpoint = sp;
}

void Indicator::setLowLimit(unsigned int l, int color){
	this->lowLimit = l;
	this->lowLimitColor = color;
}

void Indicator::setSP(unsigned int sp, int color){
	this->setpoint = sp;
	this->setpointColor = color;
}


void Indicator::setNum(int num){
	clear();
	char numChar[BUF_SIZE];
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
		buf[chars-1-j] = '0'+numChar[j];
		//Serial.print("buf[i] ");Serial.println(buf[j]);
	}
	
	buf[chars]=0;
}

unsigned char Indicator::getTextLength(char* c){
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


//Overriden Methods

bool Indicator::checkTouch(Point* p){
	return true;
}

bool Indicator::isButton(){
	return false;
}

/*
void Indicator::show(){
	update();
}

void Indicator::update(){
	
}
*/
