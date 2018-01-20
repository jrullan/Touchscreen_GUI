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
	//Tft.fillRect(x,y,yScaleWidth-borderWidth+1,h+borderWidth,bgColor);
	
	// Draw Max value and line
	setNum(scaleMax); // Sets buf with scaleMax value
	Tft.drawString(buf,x,getYVal(scaleMax),1,borderColor);
	Tft.drawHorizontalLine(lineX,getYVal(scaleMax),10,borderColor);
	
	// Draw Setpoint value and line
	setNum(setpoint); // Sets buf with setpoint value
	Tft.drawString(buf,x,getYVal(setpoint)-(FONT_Y/2),1,setpointColor);
	Tft.drawHorizontalLine(lineX,getYVal(setpoint),10,setpointColor);
	
	// Draw Min value and line
	setNum(scaleMin); // Sets buf with scaleMin value
	Tft.drawString(buf,x,getYVal(scaleMin)-(FONT_Y),1,borderColor);
	Tft.drawHorizontalLine(lineX,getYVal(scaleMin),10,borderColor);

	// Draw HI and LOW limits values and lines
	if(hiLimit < scaleMax){
		setNum(hiLimit);
		Tft.drawString(buf,x,getYVal(hiLimit)-(FONT_Y/2),1,hiLimitColor);
		Tft.drawHorizontalLine(lineX,getYVal(hiLimit),10,hiLimitColor);
	}
	if(lowLimit > scaleMin){
		setNum(lowLimit);
		Tft.drawString(buf,x,getYVal(lowLimit)-(FONT_Y/2),1,lowLimitColor);
		Tft.drawHorizontalLine(lineX,getYVal(lowLimit),10,lowLimitColor);
	}	
//=======================================
	/*
	unsigned int val;
	//Draw SCALE
	int textWidth = 4 * FONT_SPACE;

	Tft.fillRect(x+textWidth,y+borderWidth,10,h,BLACK);

	setNum(scaleMax);
	Tft.drawString(buf,x,y,1,borderColor);
	Tft.drawHorizontalLine(x+textWidth,y+borderWidth,10,borderColor);

	setNum(setpoint);
	val = map(setpoint,scaleMin,scaleMax,h-borderWidth,borderWidth);
	Tft.drawString(buf,x+FONT_SPACE,y+(val)-(FONT_Y>>1)-borderWidth,1,setpointColor);
	Tft.drawHorizontalLine(x+textWidth,y+(val)-borderWidth,10,setpointColor);
		
	setNum(scaleMin);
	Tft.drawString(buf,x+2*FONT_SPACE,y+h-(FONT_Y),1,borderColor);
	Tft.drawHorizontalLine(x+textWidth,y+(h)-borderWidth,10,borderColor);

	if(hiLimit != scaleMax){// && currentValue > lowLimit){
		val = map(hiLimit,scaleMin,scaleMax,h-borderWidth,borderWidth);
		Tft.drawHorizontalLine(x+textWidth,y+val,10,hiLimitColor);
		//Tft.drawHorizontalLine(x+borderWidth+textWidth,y+val,w-(2*borderWidth),hiLimitColor);
	}
	if(lowLimit != scaleMin){// && currentValue < hiLimit){
		val = map(lowLimit,scaleMin,scaleMax,h-borderWidth,borderWidth);
		Tft.drawHorizontalLine(x+textWidth,y+val,10,lowLimitColor);
		//Tft.drawHorizontalLine(x+borderWidth+textWidth,y+val,w-(2*borderWidth),lowLimitColor);
	}
	*/
	//drawLimits();
}

int Gauge::getYVal(int value){
	int yBase = y ;//+ borderWidth ;//+ 1; 	// +1 to account for trend width line
	int maxHeight = h-borderWidth; // -2 to account for trend line width
	unsigned int val = constrain(value,scaleMin,scaleMax);
	val = map(val,scaleMin,scaleMax,maxHeight,borderWidth+1); //-1 to account for trend line width
	return yBase + val;
}

//Overriden virtual methods
void Gauge::show(){
	if(this->visible){
		Tft.fillRect(x,y,w,h,bgColor);
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
		Tft.drawRect(xPos++,yPos++,width--,height--,borderColor);
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
	Tft.fillRect(x+borderWidth+yScaleWidth, y+borderWidth, w-(2*borderWidth), val-1,bgColor);
	//--bar fill
	int color=fgColor;
	if(currentValue > this->hiLimit) color = hiLimitColor;
	if(currentValue < this->lowLimit) color = lowLimitColor;
	Tft.fillRect(x+borderWidth+yScaleWidth, y+val, w-(2*borderWidth),h-val-borderWidth,color);	
}

void Gauge::update(){
	//if(previousValue == currentValue) return;
	drawFill();
}
