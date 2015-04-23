#include "NewGauge.h"
//Constructor
NewGauge::NewGauge(){
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
}

NewGauge::NewGauge(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit)
{	
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
	this->setSize(width,height);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->setColors(BLACK,BLUE,WHITE);
	init();
}

NewGauge::~NewGauge(){}

//Methods
void NewGauge::init(){
	Indicator::init();
	x=0;
	y=0;
	borderWidth=2;
	this->hiLimit = scaleMax;
	this->lowLimit = scaleMin;
	this->currentValue = setpoint;
}

void NewGauge::clear(){
	for(int i = BUF_SIZE-1; i >= 0; i--)
	{
			buf[i] = 0;
	}
}

void NewGauge::drawLimits(){
}

void NewGauge::drawScale(){
	unsigned int val;
	//Draw SCALE
	int textWidth = 4 * FONT_SPACE;
	
	Tft.fillRectangle(x+textWidth,y+borderWidth,10,h,BLACK);
	
	setNum(scaleMax);
	Tft.drawString(buf,x,y,1,borderColor);
	Tft.drawHorizontalLine(x+textWidth,y+borderWidth,10,borderColor);
	
	setNum(setpoint);
	val = map(setpoint,scaleMin,scaleMax,h-borderWidth,borderWidth);
	Tft.drawString(buf,x+FONT_SPACE,y+(val)-(FONT_Y>>1)-borderWidth,1,fgColor);
	Tft.drawHorizontalLine(x+textWidth,y+(val)-borderWidth,10,fgColor);
		
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
	//drawLimits();
}

//Overriden virtual methods
void NewGauge::show(){
	//Serial.print("Show");Serial.println();
  this->drawScale();
  this->update();
}

void NewGauge::update(){
	int textWidth = 4 * FONT_SPACE + 10;
	int xPos = x + textWidth;
	int width = w;
  byte yPos = y;
  byte height = h;
  unsigned int val;
  
	//Draw the Vertical Bar
  //--border
  for(byte i=borderWidth; i!=0;i--){
    Tft.drawRectangle(xPos++,yPos++,width--,height--,borderColor);
    width--;
    height--;
  }
  val = map(currentValue,scaleMin,scaleMax,h-borderWidth,borderWidth);
	//--background fill
	Tft.fillRectangle(x+borderWidth+textWidth, y+borderWidth, w-(2*borderWidth), val-1,bgColor);
	//--bar fill
	int color=fgColor;
	if(currentValue > this->hiLimit) color = hiLimitColor;
	if(currentValue < this->lowLimit) color = lowLimitColor;
  Tft.fillRectangle(x+borderWidth+textWidth, y+val, w-(2*borderWidth),h-val-borderWidth,color);
  
}
