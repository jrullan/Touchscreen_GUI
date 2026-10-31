#include "Gauge.h"
//Constructor
Gauge::Gauge(){
	init();
}

Gauge::Gauge(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit)
{	
	this->setSize(width,height);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->hiLimit = scaleMax;
	this->lowLimit = scaleMin;
	this->currentValue = this->setpoint;
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
	Indicator::clear();
}

void Gauge::drawLimits(){
}

void Gauge::drawScale(){
	if(!canDraw() || !buf) return;
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
	return getYValValue(value);
}

int Gauge::getYVal(unsigned int value){
	return getYValValue(value);
}

int Gauge::getYValValue(int64_t value){
	const int last = h > 0 ? h-1 : 0;
	const int top = constrain(borderWidth+1, 0, last);
	const int bottom = constrain(h-borderWidth, top, last);
	return y + mapScale(value, bottom, top);
}

//Overriden virtual methods
void Gauge::show(){
	visible = true;
	if(!canDraw() || !buf) return;
	if(this->visible){
		myCanvas->tft->fillRect(x,y,w,h,bgColor);
		this->drawScale();
		this->drawBorder();
		this->update();
	}
}

void Gauge::drawBorder(){
	if(!canDraw() || !buf) return;
	int xPos = x + yScaleWidth;
	int width = w;
	int yPos = y;
	int height = h;
	
	//Draw the Vertical Bar
	//--border
	for(int i=borderWidth; i>0 && width>0 && height>0; --i){
		myCanvas->tft->drawRect(xPos++,yPos++,width--,height--,borderColor);
		width--;
		height--;
	}
}

void Gauge::drawFill(){
	if(!canDraw() || !buf || w <= 0 || h <= 0) return;
	const int bw = borderWidth > 0 ? borderWidth : 0;
	const int width = w-2*bw, bottom = h-bw;
	if(width <= 0 || bottom <= bw) return;
	const int val = mapScale(currentValue, bottom, bw);
	if(val > bw) myCanvas->tft->fillRect(x+bw+yScaleWidth,y+bw,width,val-bw,bgColor);
	int color=fgColor;
	if(currentValue >= hiLimit) color = hiLimitColor;
	if(currentValue <= lowLimit) color = lowLimitColor;
	if(bottom > val) myCanvas->tft->fillRect(x+bw+yScaleWidth,y+val,width,bottom-val,color);
}

void Gauge::update(){
	if(!canDraw() || !buf) return;
	drawFill();
}
