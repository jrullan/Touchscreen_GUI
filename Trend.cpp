#include "Trend.h"
//Constructor
Trend::Trend(){
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
}

Trend::Trend(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit)
{	
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
		
	this->setSize(width,height);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->setColors(BLACK,BLUE,WHITE);
	init();
}

Trend::~Trend(){}

//Methods
void Trend::init(){
	Indicator::init();
	
	// Reserve memory for trend values
	if(values = (uint8_t *)malloc(MAX_TREND_VALUES)){ 
		memset(values,0,MAX_TREND_VALUES*sizeof(uint8_t));
		//Serial.println("Memory allocated for trend values");
	}else{
		//Serial.println("Memory was not allocated for trend values");
	}
	
	borderWidth=2;
	this->hiLimit = scaleMax;
	this->lowLimit = scaleMin;
	this->currentValue = setpoint;
}

void Trend::clear(){
	for(int i = BUF_SIZE-1; i >= 0; i--)
	{
			buf[i] = 0;
	}
}

/*
int Trend::limit(int val, int min, int max){
	if(val > max) val = max;
	if(val < min) val = min;
	return val;
}
*/

void Trend::drawYScale(){
	//Draw Y SCALE
	int textWidth = 4 * FONT_X;
	int lineX = x + textWidth - 1; //to separate from border
	//int textY;
	//int minY = getYVal(scaleMax);	
	//int maxY = getYVal(scaleMin + FONT_Y/2);

	// Draw background
	Tft.fillRectangle(x,y,yScaleWidth-borderWidth+1,h-xScaleHeight+borderWidth,bgColor);
	
	// Draw Max value and line
	setNum(scaleMax); // Sets buf with scaleMax value
	//textY = getYVal(scaleMax);
	Tft.drawString(buf,x,getYVal(scaleMax),1,borderColor);
	Tft.drawHorizontalLine(lineX,getYVal(scaleMax),10,borderColor);
	
	// Draw Setpoint value and line
	setNum(setpoint); // Sets buf with setpoint value
	//textY = getYVal(setpoint)-(FONT_Y/2);
	Tft.drawString(buf,x,getYVal(setpoint)-(FONT_Y/2),1,setpointColor);
	Tft.drawHorizontalLine(lineX,getYVal(setpoint),10,setpointColor);
	
	// Draw Min value and line
	setNum(scaleMin); // Sets buf with scaleMin value
	//textY = getYVal(scaleMin)-(FONT_Y);
	Tft.drawString(buf,x,getYVal(scaleMin)-(FONT_Y),1,borderColor);
	Tft.drawHorizontalLine(lineX,getYVal(scaleMin),10,borderColor);

	// Draw HI and LOW limits values and lines
  if(hiLimit < scaleMax){
	  setNum(hiLimit);
	  //textY = getYVal(hiLimit)-(FONT_Y/2);
	  Tft.drawString(buf,x,getYVal(hiLimit)-(FONT_Y/2),1,hiLimitColor);
		Tft.drawHorizontalLine(lineX,getYVal(hiLimit),10,hiLimitColor);
  }
  if(lowLimit > scaleMin){
	  setNum(lowLimit);
	  //textY = getYVal(lowLimit)-(FONT_Y/2);
	  Tft.drawString(buf,x,getYVal(lowLimit)-(FONT_Y/2),1,lowLimitColor);
		Tft.drawHorizontalLine(lineX,getYVal(lowLimit),10,lowLimitColor);
  }
}

void Trend::drawXScale(){
	unsigned int val;
	int textWidth = 4 * FONT_SPACE;
	
	//Draw X Scale
	//--background
	int xPos = x + yScaleWidth + 1;
	int yPos = y + h - xScaleHeight;
	int xWidth = w - yScaleWidth;
	int xHeight = h - xScaleHeight;
	Tft.fillRectangle(xPos,yPos,xWidth,xHeight,bgColor);
	yPos += borderWidth;

	//--right
	setNum(0);
	Tft.drawString(buf,xPos+xWidth-FONT_X,yPos+10+FONT_Y,1,borderColor);

	xWidth -= borderWidth;
	int xp = xPos + xWidth;
	
	for(int i = 0; i < MAX_TREND_VALUES+1; i++){
		xp = xPos + xWidth - i*xWidth/(MAX_TREND_VALUES);
		Tft.drawVerticalLine(xp,yPos,10,borderColor);	
		//draw alternating numbers
		if(i>0 && !(i%2)){
			setNum(map(i,0,MAX_TREND_VALUES,0,maxX));
			byte size = getTextLength(buf);
			Tft.drawString(buf,xp - size*(FONT_X>>1),yPos+10+FONT_Y,1,borderColor);	
		}
	}
	
}

void Trend::drawValues(INT16U color){
	unsigned int val; //= map(currentValue,scaleMin,scaleMax,h-borderWidth,borderWidth);
	int x1, y1, x2, y2;
	
	//-- loop through all values and plot them left to right (starting with values[7])
	for(int i = MAX_TREND_VALUES; i!=0; i--){
		int j = i-1;
		
		//-- previous values[j] coordinates 2-previous 1-current
		x2 = x1;
		y2 = y1;
		
		x1 = getXVal(j);
		y1 = getYVal(values[j]);
		
		if(j<MAX_TREND_VALUES-1){
			Tft.drawLine(x2,y2-1,x1,y1-1,color);
			Tft.drawLine(x2,y2,x1,y1,color);
			Tft.drawLine(x2,y2+1,x1,y1+1,color);
		}
	}
}

void Trend::drawBorder(){
	int xPos = x + yScaleWidth;
  int yPos = y + borderWidth;
	int width = w-yScaleWidth-2*borderWidth;
  int borderW, borderH;
  borderW = w - yScaleWidth;
  borderH = h - xScaleHeight;
  
  //--border
  for(byte i=borderWidth; i!=0; i--){
    Tft.drawRectangle(xPos,yPos,borderW,borderH,borderColor);
    xPos++;
    yPos++;
    borderW -= 2;
    borderH -= 2;
  }	
}

void Trend::drawThresholdLines(bool colors){
	int xPos = x + borderWidth + yScaleWidth - 1;
	int width = w-yScaleWidth-2*borderWidth;
  
	if(hiLimit < scaleMax){
		Tft.drawHorizontalLine(xPos+1,getYVal(hiLimit),width,!colors?bgColor:hiLimitColor);
	}
	
	Tft.drawHorizontalLine(xPos+1,getYVal(setpoint),width,!colors?bgColor:setpointColor);
	
	if(lowLimit > scaleMin){
		Tft.drawHorizontalLine(xPos+1,getYVal(lowLimit),width,!colors?bgColor:lowLimitColor);
	}
}

/* 
 * Sets maximum X value in milli-seconds. This value is used to draw the interval values
 * of the x-scale. If the value is too high, it automatically adjusts
 * to minutes.
 */
void Trend::setMaxX(int m){
	int interval = m/MAX_TREND_VALUES;
	if(interval*2 >= 60){ //-- auto adjust for minutes interval if too many seconds.
		maxX = m/60;
	}else{
		maxX = m;
	}
}

int Trend::getXVal(int index){
	int xBase = x+yScaleWidth+borderWidth;
	int effWidth = w - yScaleWidth - 2*borderWidth;
	int inc = index*(effWidth)/(MAX_TREND_VALUES-1);
	return xBase + effWidth - inc;
}

int Trend::getYVal(int value){
	int yBase = y + borderWidth + 1; 	// +1 to account for trend width line
	int maxHeight = h-xScaleHeight-borderWidth - 2; // -2 to account for trend line width
	unsigned int val = constrain(value,scaleMin,scaleMax);
	val = map(val,scaleMin,scaleMax,maxHeight,borderWidth+1); //-1 to account for trend line width
	return yBase + val;
}

int Trend::getMin(){
	int val = values[0];
	for(int i = 1; i<MAX_TREND_VALUES; i++){
		if(values[i] < val) val = values[i];
	}
	return val;
}

int Trend::getMax(){
	int val = values[0];
	for(int i = 1; i<MAX_TREND_VALUES; i++){
		if(values[i] > val) val = values[i];
	}
	return val;
}

void Trend::autoFit(bool scale){
	this->drawValues(this->bgColor);	//erase original line
	this->drawThresholdLines(false); //erase threshold lines	
	
	if(scale){
		int inc = 2;
		int min = getMin();
		int max = getMax();
		
		if(min <= scaleMin && min != 0){ 
			scaleMin = min-inc;
		}else{
			if(min-scaleMin > inc) scaleMin = min - inc;
		}
		
		if(max >= scaleMax){
			scaleMax = max+inc;
		}else{
			if(scaleMax-max > inc) scaleMax = max + inc;
		}
	}
	
	drawYScale();
	
	this->drawThresholdLines(true);	
  
  this->drawValues(this->fgColor);	
}

// Adds a new value to the trend
void Trend::addValue(uint8_t val){
	previousValue = currentValue;
	currentValue = val;

	//--delete previous line (bgColor)
	if(visible) drawValues(this->bgColor);

	//--push value into the array
	for(int i = MAX_TREND_VALUES; i!=0; i--){

		if(i==1){
			values[i-1] = val;
		}else{
			values[i-1] = values[i-2];
		}
		//Serial.print("Value[");
		//Serial.print(i-1);
		//Serial.print("] = ");
		//Serial.println(values[i-1]);
	}	
	
	update();
}

//Overriden virtual methods
void Trend::show(){
	//Serial.print("Show");Serial.println();
  this->drawYScale();
  this->drawXScale();
  this->drawBorder();
  this->update();
}

void Trend::update(){
	if(!forcedUpdate){	
		if(!visible) return;
	}
	
	if(enableAutoFit){
		if(values[0] >= scaleMax - 2 || values[0] <= scaleMin - 2){
			autoFit(true);
			return;
		}
		/*
		 *if(updates++ > MAX_TREND_VALUES){
			autoFit();
			updates = 0;
			return;
		}*/
	}
	
	this->drawThresholdLines(true);
	this->drawValues(this->fgColor);	
}
