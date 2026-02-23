#include "Dial.h"

// Integer sine lookup table: sin(0..90) * 1024, stored in flash
static const int16_t SIN_LUT[] PROGMEM = {
	   0,   18,   36,   54,   71,   89,  107,  125,  143,  160,
	 178,  195,  213,  230,  248,  265,  282,  299,  316,  333,
	 350,  367,  384,  400,  416,  433,  449,  465,  481,  496,
	 512,  527,  543,  558,  573,  588,  602,  616,  631,  644,
	 658,  672,  685,  699,  711,  724,  737,  749,  761,  773,
	 785,  796,  807,  818,  829,  839,  849,  859,  868,  878,
	 887,  896,  904,  912,  920,  928,  935,  943,  950,  956,
	 962,  968,  974,  979,  984,  989,  994,  998, 1002, 1005,
	1009, 1012, 1014, 1016, 1018, 1020, 1022, 1023, 1023, 1024,
	1024
};

static int fastSin(int deg) {
	deg = deg % 360;
	if(deg < 0) deg += 360;
	if(deg <= 90)  return (int16_t)pgm_read_word(&SIN_LUT[deg]);
	if(deg <= 180) return (int16_t)pgm_read_word(&SIN_LUT[180 - deg]);
	if(deg <= 270) return -(int16_t)pgm_read_word(&SIN_LUT[deg - 180]);
	return -(int16_t)pgm_read_word(&SIN_LUT[360 - deg]);
}

static int fastCos(int deg) {
	return fastSin(deg + 90);
}

//Constructor
Dial::Dial(){
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1);
}

Dial::Dial(unsigned int radius, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit)
{
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1);
	this->setSize(radius);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->setColors(BLACK,BLUE,WHITE);
	init();
}

Dial::~Dial(){}

//Methods
void Dial::init(){
	Indicator::init();
	type = 0x21;
	this->hiLimit = scaleMax;
	this->lowLimit = scaleMin;
	this->currentValue = scaleMin;
	this->maxDegree = 315;
	this->minDegree = 585;
	this->tickSize = 10;
	this->gap = 5;
	this->tickDegree = 45;
	this->showVal = true;
	this->showTicks = true;
}

void Dial::clear(){
	for(int i = BUF_SIZE-1; i >= 0; i--)
	{
			buf[i] = 0;
	}
}

void Dial::setSize(int radius){
	this->w = radius*2;
	this->h = radius*2;
	this->radius = radius;
}

void Dial::drawBorder(){
	int color = this->fgColor;
	/*
	if(currentValue >= hiLimit) color = hiLimitColor;
	if(currentValue <= lowLimit) color = lowLimitColor;

	/*
	for(int i=0; i < this->borderWidth; i++)
	{
  	myCanvas->tft->drawCircle(x,y,radius-i,color);
  }
	*/
	myCanvas->tft->fillCircle(x,y,radius,color);
	//myCanvas->tft->fillCircle(x,y,radius-borderWidth,bgColor);
	//drawFace();
}

void Dial::drawFace(){
	// Draw face
	myCanvas->tft->fillCircle(x,y,radius - this->borderWidth,this->bgColor);

	// Draw border
	//drawBorder();

	int X1,Y1,X2,Y2;

  // Draw ticks
	if(showTicks){
		for(int i=maxDegree; i<=minDegree; i+=tickDegree)
		{
			X1 = getX(x,i,radius-tickSize);
			Y1 = getY(y,i,radius-tickSize);
			X2 = getX(x,i,radius-borderWidth);
			Y2 = getY(y,i,radius-borderWidth);
			myCanvas->tft->drawLine(X1,Y1,X2,Y2,borderColor);
		}
	}else{
		int i = minDegree;
		X1 = getX(x,i,radius-tickSize);
		Y1 = getY(y,i,radius-tickSize);
		X2 = getX(x,i,radius-borderWidth);
		Y2 = getY(y,i,radius-borderWidth);
		myCanvas->tft->drawLine(X1,Y1,X2,Y2,borderColor);

		i = maxDegree;
		X1 = getX(x,i,radius-tickSize);
		Y1 = getY(y,i,radius-tickSize);
		X2 = getX(x,i,radius-borderWidth);
		Y2 = getY(y,i,radius-borderWidth);
		myCanvas->tft->drawLine(X1,Y1,X2,Y2,borderColor);
	}

	// Draw Setpoint line
	if(setpoint){
		int i = map(setpoint,scaleMin,scaleMax,minDegree,maxDegree);
		X1 = getX(x,i,radius-tickSize);
		Y1 = getY(y,i,radius-tickSize);
		X2 = getX(x,i,radius-borderWidth);
		Y2 = getY(y,i,radius-borderWidth);
		myCanvas->tft->drawLine(X1,Y1,X2,Y2,setpointColor);
	}

	// Draw High limit line
	if(hiLimit < scaleMax){
		int i = map(hiLimit,scaleMin,scaleMax,minDegree,maxDegree);
		X1 = getX(x,i,radius-tickSize);
		Y1 = getY(y,i,radius-tickSize);
		X2 = getX(x,i,radius-borderWidth);
		Y2 = getY(y,i,radius-borderWidth);
		myCanvas->tft->drawLine(X1,Y1,X2,Y2,hiLimitColor);
	}

	// Draw Low Limit line
	if(lowLimit > scaleMin){
		int i = map(lowLimit,scaleMin,scaleMax,minDegree,maxDegree);
		X1 = getX(x,i,radius-tickSize);
		Y1 = getY(y,i,radius-tickSize);
		X2 = getX(x,i,radius-borderWidth);
		Y2 = getY(y,i,radius-borderWidth);
		myCanvas->tft->drawLine(X1,Y1,X2,Y2,lowLimitColor);
	}

	// Draw min value
	setNum(scaleMin);
	myCanvas->tft->drawString(buf,x-radius+FONT_SPACE,y+radius-FONT_Y,1,borderColor);

	// Draw max value
	setNum(scaleMax);
	myCanvas->tft->drawString(buf,getX(x,maxDegree,radius-tickSize),y+radius-FONT_Y,1,borderColor);

}

void Dial::drawNeedle(int cX, int cY, int degree, int radius, int color){
	degree = map(constrain(degree,scaleMin,scaleMax),scaleMin,scaleMax,585,315);

	int s = fastSin(degree);
	int c = fastCos(degree);

	// Trig identities: cos(d-90)=sin(d), sin(d-90)=-cos(d), cos(d+90)=-sin(d), sin(d+90)=cos(d)
	int pX1 = cX + (4 * s) / 1024;
	int pY1 = cY + (4 * c) / 1024;
	int pX2 = cX - (4 * s) / 1024;
	int pY2 = cY - (4 * c) / 1024;
	int pX3 = cX + (radius * c) / 1024;
	int pY3 = cY - (radius * s) / 1024;

	myCanvas->tft->fillTriangle(pX1,pY1,pX2,pY2,pX3,pY3,color);
	myCanvas->tft->fillCircle(cX, cY, 4, color);
}

void Dial::drawNeedleAndValue(){
	int color = fgColor;
	if(currentValue >= hiLimit) color = hiLimitColor;
	if(currentValue <= lowLimit) color = lowLimitColor;

	if(showVal && currentValue != previousValue){
		int fontSize = 2;
		int dSpace;
		if(currentValue < 10)   dSpace = 3 * fontSize;
		else if(currentValue < 100)  dSpace = 6 * fontSize;
		else if(currentValue < 1000) dSpace = 9 * fontSize;
		else                         dSpace = 12 * fontSize;
		int valY = y + radius - 16 * fontSize;
		myCanvas->tft->fillRect(x - 12*fontSize, valY, 24*fontSize, 8*fontSize, bgColor);
		myCanvas->tft->drawNumber(currentValue, x - dSpace, valY, fontSize, color);
	}

	// Erase old needle by redrawing it in bgColor, then draw new needle
	drawNeedle(x, y, previousValue, radius - tickSize - gap, bgColor);
	drawNeedle(x, y, currentValue, radius - tickSize - gap, color);
}

int Dial::getX(int cX,int deg, int radius){
	return cX + (radius * fastCos(deg)) / 1024;
}

int Dial::getY(int cY, int deg, int radius){
	return cY - (radius * fastSin(deg)) / 1024;
}


//Overriden virtual methods
void Dial::show(){
	myCanvas->tft->startWrite();
	drawBorder();
	drawFace();
	drawNeedleAndValue();
	myCanvas->tft->endWrite();
}

void Dial::update(){
	if(!visible) return;
	if(!forcedUpdate && !_dirty) return;
	_dirty = false;
	myCanvas->tft->startWrite();
	drawNeedleAndValue();
	myCanvas->tft->endWrite();
}
