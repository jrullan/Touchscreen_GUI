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
	x=0;
	y=0;
	borderWidth=3;
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

unsigned int Dial::getCV(){
	return currentValue;
}

void Dial::setLimits(unsigned int min, unsigned int sp, unsigned int max){
	this->scaleMin = min;
	this->setpoint = sp;
	this->scaleMax = max;
}

void Dial::setHiLimit(unsigned int h, int color){
	//Serial.print("Hi limit set to : ");Serial.println(h);
	this->hiLimit = h;
	this->hiLimitColor = color;
}

void Dial::setLowLimit(unsigned int l, int color){
	this->lowLimit = l;
	this->lowLimitColor = color;
}

void Dial::setCV(int cv){
	//Serial.print("CV: ");Serial.println(cv);
	previousValue = currentValue;
	currentValue = constrain(cv,scaleMin,scaleMax);
	update();
}

void Dial::setNum(int num){
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

void Dial::setSize(int radius){
	this->w = radius*2;
	this->h = radius*2;
	this->radius = radius;
}

void Dial::drawBorder(){
	int color = fgColor;
	if(currentValue > hiLimit) color = hiLimitColor;
	if(currentValue < lowLimit) color = lowLimitColor;

	for(int i=0; i<borderWidth; i++)
	{
  	Tft.drawCircle(x,y,radius-i,color);
  	//Serial.print("Drawing border with r = ");Serial.println(radius-i);
  }
}

void Dial::drawFace(){
	// Draw face
	Tft.fillCircle(x,y,radius,bgColor);
	
	// Draw border  
  drawBorder();

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
	
  if(setpoint){
  	int i = map(setpoint,scaleMin,scaleMax,minDegree,maxDegree);
    X1 = getX(x,i,radius-tickSize);
    Y1 = getY(y,i,radius-tickSize);
    X2 = getX(x,i,radius-borderWidth);
    Y2 = getY(y,i,radius-borderWidth);    
    Tft.drawLine(X1,Y1,X2,Y2,fgColor);
	} 
	  
  if(hiLimit < scaleMax){
  	int i = map(hiLimit,scaleMin,scaleMax,minDegree,maxDegree);
    X1 = getX(x,i,radius-tickSize);
    Y1 = getY(y,i,radius-tickSize);
    X2 = getX(x,i,radius-borderWidth);
    Y2 = getY(y,i,radius-borderWidth);    
    Tft.drawLine(X1,Y1,X2,Y2,hiLimitColor);
	}  	
	
  if(lowLimit > scaleMin){
  	int i = map(lowLimit,scaleMin,scaleMax,minDegree,maxDegree);
    X1 = getX(x,i,radius-tickSize);
    Y1 = getY(y,i,radius-tickSize);
    X2 = getX(x,i,radius-borderWidth);
    Y2 = getY(y,i,radius-borderWidth);    
    Tft.drawLine(X1,Y1,X2,Y2,lowLimitColor);
	}  	  

}

void Dial::drawNeedle(int cX, int cY, int degree, int radius, int color){
  degree = map(constrain(degree,scaleMin,scaleMax),scaleMin,scaleMax,585,315);
  
  int pX1,pY1,pX2,pY2,pX3,pY3;
  
  pX1 = getX(cX,degree-90,2);
  pY1 = getY(cY,degree-90,2);
  pX2 = getX(cX,degree+90,2);
  pY2 = getY(cY,degree+90,2);
  pX3 = getX(cX,degree,radius);
  pY3 = getY(cY,degree,radius);
  // Outer triangle
  Tft.drawLine(pX1,pY1,pX2,pY2,color);
  Tft.drawLine(pX1,pY1,pX3,pY3,color);
  Tft.drawLine(pX2,pY2,pX3,pY3,color);

  pX1 = getX(cX,degree-90,1);
  pY1 = getY(cY,degree-90,1);
  pX2 = getX(cX,degree+90,1);
  pY2 = getY(cY,degree+90,1);
  
  // Inner Triangle
  Tft.drawLine(pX1,pY1,pX2,pY2,color);
  Tft.drawLine(pX1,pY1,pX3,pY3,color);
  Tft.drawLine(pX2,pY2,pX3,pY3,color);  
  
  // Center Line
  Tft.drawLine(cX,cY,pX3,pY3,color);
}

int Dial::getX(int cX,int deg, int radius){
  return (cX + radius * cos(deg*PI/180));
}

int Dial::getY(int cY, int deg, int radius){
  return (cY - radius * sin(deg*PI/180));
}


//Overriden virtual methods

bool Dial::checkTouch(Point* p){
	return true;
}

bool Dial::isButton(){
	return false;
}

void Dial::show(){
	// Draw face
	drawFace();
	update();

/*
	//Serial.print("Show");Serial.println();
	unsigned int val;
  int xPos = x;	
  int width = w;
  byte yPos = y;
  byte height = h;
	
	//Draw SCALE
	int textWidth = 4 * FONT_SPACE;
	
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

  if(hiLimit != scaleMax && currentValue > lowLimit){
	  val = map(hiLimit,scaleMin,scaleMax,h-borderWidth,borderWidth);
		Tft.drawHorizontalLine(x+textWidth,y+val,10,hiLimitColor);
  	//Tft.drawHorizontalLine(x+borderWidth+textWidth,y+val,w-(2*borderWidth),hiLimitColor);
  }
  if(lowLimit != scaleMin && currentValue < hiLimit){
	  val = map(lowLimit,scaleMin,scaleMax,h-borderWidth,borderWidth);
		Tft.drawHorizontalLine(x+textWidth,y+val,10,lowLimitColor);
  	//Tft.drawHorizontalLine(x+borderWidth+textWidth,y+val,w-(2*borderWidth),lowLimitColor);
  }
  
  
  //Draw the Vertical Bar
	textWidth += 10;
	xPos = x + textWidth;
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
	if(currentValue >= this->hiLimit) color = hiLimitColor;
	if(currentValue <= this->lowLimit) color = lowLimitColor;
  Tft.fillRectangle(x+borderWidth+textWidth, y+val, w-(2*borderWidth),h-val-borderWidth,color);
  */
}

void Dial::update(){
	int color = fgColor;
	
	// Limit crossing forces border to redraw
	if(previousValue <= hiLimit && previousValue >= lowLimit){
		if(currentValue > hiLimit || currentValue < lowLimit) drawBorder();
	}
	if(previousValue > hiLimit){
		if(currentValue <= hiLimit) drawBorder();
	}
	if(previousValue < lowLimit){
		if(currentValue >= lowLimit) drawBorder();
	}
	
	// Draw needle
	drawNeedle(x,y,previousValue,radius-tickSize-gap,bgColor);
	if(currentValue > hiLimit) color = hiLimitColor;
	if(currentValue < lowLimit) color = lowLimitColor;
	drawNeedle(x,y,currentValue,radius-tickSize-gap,color);
	
	if(showVal){
		// Draw current value
		int dSpace;
		int fontSize = 2;
  	if(currentValue<10) dSpace = 3 * fontSize;
  	if(currentValue>=10) dSpace = 6 * fontSize;
  	if(currentValue>99) dSpace = 9 * fontSize;
  	Tft.fillRectangle(x-9*fontSize,y+radius-12*fontSize,18*fontSize,8*fontSize,bgColor);
  	Tft.drawNumber(currentValue,x-dSpace,y+radius-12*fontSize,fontSize,color);
	}
}
