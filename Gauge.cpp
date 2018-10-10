#include "Gauge.h"
//Constructor
Gauge::Gauge(){
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
}

Gauge::Gauge(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit)
{	
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
	this->setSize(width,height);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->setColors(BLACK,BLUE,WHITE);
	init();
}

Gauge::~Gauge(){}

//Methods
void Gauge::init(){
	Indicator::init();
	type = 0x22;
	borderWidth=2;
	if(this->hiLimit == 0) this->hiLimit = scaleMax;
	if(this->lowLimit == 0)this->lowLimit = scaleMin;
	if(this->currentValue == 0)this->currentValue = setpoint;
}

void Gauge::clear(){
	for(int i = BUF_SIZE-1; i >= 0; i--)
	{
			buf[i] = 0;
	}
}

void Gauge::drawLimits(){
}

void Gauge::drawScale(){
//=======================================
	//Draw Y SCALE
	int textWidth = 3 * FONT_X;
	int lineX = x + textWidth - 1; //to separate from border
	
	// Draw background
	//myCanvas->tft->fillRect(x,y,yScaleWidth-borderWidth+1,h+borderWidth,bgColor);
	
	// Draw Max value and line
	setNum(scaleMax); // Sets buf with scaleMax value
	myCanvas->tft->drawString(buf,x,getYVal(scaleMax),1,borderColor);
	myCanvas->tft->drawHorizontalLine(lineX,getYVal(scaleMax),10,borderColor);
	
	// Draw Setpoint value and line
	setNum(setpoint); // Sets buf with setpoint value
	myCanvas->tft->drawString(buf,x,getYVal(setpoint)-(FONT_Y/2),1,setpointColor);
	myCanvas->tft->drawHorizontalLine(lineX,getYVal(setpoint),10,setpointColor);
	
	// Draw Min value and line
	setNum(scaleMin); // Sets buf with scaleMin value
	myCanvas->tft->drawString(buf,x,getYVal(scaleMin)-(FONT_Y),1,borderColor);
	myCanvas->tft->drawHorizontalLine(lineX,getYVal(scaleMin),10,borderColor);

	// Draw HI and LOW limits values and lines
	if(hiLimit < scaleMax){
		setNum(hiLimit);
		myCanvas->tft->drawString(buf,x,getYVal(hiLimit)-(FONT_Y/2),1,hiLimitColor);
		myCanvas->tft->drawHorizontalLine(lineX,getYVal(hiLimit),10,hiLimitColor);
	}
	if(lowLimit > scaleMin){
		setNum(lowLimit);
		myCanvas->tft->drawString(buf,x,getYVal(lowLimit)-(FONT_Y/2),1,lowLimitColor);
		myCanvas->tft->drawHorizontalLine(lineX,getYVal(lowLimit),10,lowLimitColor);
	}	
}

int Gauge::getYVal(int value){
	int yBase = y ;
	int maxHeight = h-borderWidth;
	unsigned int val = constrain(value,scaleMin,scaleMax);
	val = map(val,scaleMin,scaleMax,maxHeight,borderWidth+1);
	return yBase + val;
}

//Overriden virtual methods
void Gauge::show(){
	if(this->visible){
		myCanvas->tft->fillRect(x,y,w,h,bgColor);
		this->drawScale();
		this->drawBorder();
		this->update();
	}
}

void Gauge::drawBorder(){
	int xPos = x + yScaleWidth;
	int width = w;
	byte yPos = y;
	byte height = h;
	
	//Draw the Vertical Bar
	//--border
	for(byte i=borderWidth; i!=0;i--){
		myCanvas->tft->drawRect(xPos++,yPos++,width--,height--,borderColor);
		width--;
		height--;
	}
}

void Gauge::drawFill(){
	int xPos = x + yScaleWidth;
	int width = w;
	byte yPos = y;
	byte height = h;
	unsigned int val = map(currentValue,scaleMin,scaleMax,h-borderWidth,borderWidth);

	//--background fill
	myCanvas->tft->fillRect(x+borderWidth+yScaleWidth, y+borderWidth, w-(2*borderWidth), val-1,bgColor);
	//--bar fill
	int color=fgColor;
	if(currentValue > this->hiLimit) color = hiLimitColor;
	if(currentValue < this->lowLimit) color = lowLimitColor;
	myCanvas->tft->fillRect(x+borderWidth+yScaleWidth, y+val, w-(2*borderWidth),h-val-borderWidth,color);	
}

void Gauge::update(){
	drawFill();
}
