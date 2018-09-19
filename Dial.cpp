#include "Dial.h"
//Constructor
Dial::Dial(){
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
}

Dial::Dial(unsigned int radius, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit)
{	
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
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
	this->currentValue = setpoint;
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
  	Tft.drawCircle(x,y,radius-i,color);
  }
	*/
	Tft.fillCircle(x,y,radius,color);
	//Tft.fillCircle(x,y,radius-borderWidth,bgColor);
	//drawFace();
}

void Dial::drawFace(){
	// Draw face
	Tft.fillCircle(x,y,radius - this->borderWidth,this->bgColor);
	
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
			Tft.drawLine(X1,Y1,X2,Y2,borderColor);
		}
	}else{
		int i = minDegree;
		X1 = getX(x,i,radius-tickSize);
		Y1 = getY(y,i,radius-tickSize);
		X2 = getX(x,i,radius-borderWidth);
		Y2 = getY(y,i,radius-borderWidth);    
		Tft.drawLine(X1,Y1,X2,Y2,borderColor);
		
		i = maxDegree;
		X1 = getX(x,i,radius-tickSize);
		Y1 = getY(y,i,radius-tickSize);
		X2 = getX(x,i,radius-borderWidth);
		Y2 = getY(y,i,radius-borderWidth);    
		Tft.drawLine(X1,Y1,X2,Y2,borderColor);	
	}
	
	// Draw Setpoint line
	if(setpoint){
		int i = map(setpoint,scaleMin,scaleMax,minDegree,maxDegree);
		X1 = getX(x,i,radius-tickSize);
		Y1 = getY(y,i,radius-tickSize);
		X2 = getX(x,i,radius-borderWidth);
		Y2 = getY(y,i,radius-borderWidth);    
		Tft.drawLine(X1,Y1,X2,Y2,setpointColor);
	} 
	
	// Draw High limit line
	if(hiLimit < scaleMax){
		int i = map(hiLimit,scaleMin,scaleMax,minDegree,maxDegree);
		X1 = getX(x,i,radius-tickSize);
		Y1 = getY(y,i,radius-tickSize);
		X2 = getX(x,i,radius-borderWidth);
		Y2 = getY(y,i,radius-borderWidth);    
		Tft.drawLine(X1,Y1,X2,Y2,hiLimitColor);
	}  	
	
	// Draw Low Limit line
	if(lowLimit > scaleMin){
		int i = map(lowLimit,scaleMin,scaleMax,minDegree,maxDegree);
		X1 = getX(x,i,radius-tickSize);
		Y1 = getY(y,i,radius-tickSize);
		X2 = getX(x,i,radius-borderWidth);
		Y2 = getY(y,i,radius-borderWidth);    
		Tft.drawLine(X1,Y1,X2,Y2,lowLimitColor);
	}  	  
	
	// Draw min value
	setNum(scaleMin);
	Tft.drawString(buf,x-radius+FONT_SPACE,y+radius-FONT_Y,1,borderColor);

	// Draw max value
	setNum(scaleMax);
	Tft.drawString(buf,getX(x,maxDegree,radius-tickSize),y+radius-FONT_Y,1,borderColor);

}

void Dial::drawNeedle(int cX, int cY, int degree, int radius, int color){
	degree = map(constrain(degree,scaleMin,scaleMax),scaleMin,scaleMax,585,315);

	int pX1,pY1,pX2,pY2,pX3,pY3;

	pX1 = getX(cX,degree-90,4);
	pY1 = getY(cY,degree-90,4);
	pX2 = getX(cX,degree+90,4);
	pY2 = getY(cY,degree+90,4);
	pX3 = getX(cX,degree,radius);
	pY3 = getY(cY,degree,radius);

	Tft.fillTriangle(pX1,pY1,pX2,pY2,pX3,pY3,color);
	Tft.fillCircle(cX,cY,4,color);
	/*
	// Outer triangle
	Tft.drawLine(pX1,pY1,pX2,pY2,color);
	Tft.drawLine(pX1,pY1,pX3,pY3,color);
	Tft.drawLine(pX2,pY2,pX3,pY3,color);

	pX1 = getX(cX,degree-90,2);
	pY1 = getY(cY,degree-90,2);
	pX2 = getX(cX,degree+90,2);
	pY2 = getY(cY,degree+90,2);

	// Inner Triangle
	Tft.drawLine(pX1,pY1,pX2,pY2,color);
	Tft.drawLine(pX1,pY1,pX3,pY3,color);
	Tft.drawLine(pX2,pY2,pX3,pY3,color);  

	// Center Line
	Tft.drawLine(cX,cY,pX3,pY3,color);
	*/
}

int Dial::getX(int cX,int deg, int radius){
	return (cX + radius * cos(deg*PI/180));
}

int Dial::getY(int cY, int deg, int radius){
	return (cY - radius * sin(deg*PI/180));
}


//Overriden virtual methods
void Dial::show(){
	// Draw face
	drawBorder();
	drawFace();
	update();
}

void Dial::update(){
	if(!forcedUpdate){	
		if(!visible) return;
		if(previousValue == currentValue) return;
	}
	
	int color = fgColor;
	
	// Limit crossing forces border to redraw
	/*
	if(previousValue < hiLimit && previousValue > lowLimit){
		if(currentValue >= hiLimit || currentValue <= lowLimit) drawBorder();
	}
	if(previousValue >= hiLimit){
		if(currentValue < hiLimit) drawBorder();
	}
	if(previousValue <= lowLimit){
		if(currentValue > lowLimit) drawBorder();
	}
	*/
	
	// Draw needle
	drawNeedle(x,y,previousValue,radius-tickSize-gap,bgColor);
	if(currentValue >= hiLimit) color = hiLimitColor;
	if(currentValue <= lowLimit) color = lowLimitColor;
	drawNeedle(x,y,currentValue,radius-tickSize-gap,color);
	
	if(showVal){
		// Draw current value
		int dSpace;
		int fontSize = 2;
		if(currentValue<10) dSpace = 3 * fontSize;
		if(currentValue>=10) dSpace = 6 * fontSize;
		if(currentValue>99) dSpace = 9 * fontSize;
		Tft.fillRect(x-9*fontSize,y+radius-12*fontSize,18*fontSize,8*fontSize,bgColor);
		Tft.drawNumber(currentValue,x-dSpace,y+radius-12*fontSize,fontSize,color);
	}
}
