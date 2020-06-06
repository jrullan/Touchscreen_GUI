#include "Trend.h"
//Constructor
Trend::Trend(){
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
}

Trend::Trend(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit, uint8_t max_trend_values)
{	
	if(buf = (char *)malloc(BUF_SIZE+1)) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug	
	this->setSize(width,height);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->setColors(BLACK,BLUE,WHITE);
	this->maxValues = max_trend_values;
	init();
}

Trend::~Trend(){}

//Methods
void Trend::init(){
	Indicator::init();
	type = 0x23;
	borderWidth=2;
	
	//this->maxValues = w - yScaleWidth - 2*borderWidth;
	//this->maxValues = MAX_TREND_VALUES;

	// Reserve memory for trend values
	if(values = (uint8_t *)malloc(maxValues)){ 
		memset(values,0,maxValues*sizeof(uint8_t));
		Serial.println("Memory allocated for trend values");
	}else{
		Serial.println("Memory was not allocated for trend values");
	}
	
	this->hiLimit = scaleMax;
	this->lowLimit = scaleMin;
	this->currentValue = setpoint;
	this->setWindow(0,maxValues-1);
}

void Trend::clear(){
	for(int i = BUF_SIZE-1; i >= 0; i--)
	{
			buf[i] = 0;
	}
}

void Trend::drawYScale(){
	//Draw Y SCALE
	int textWidth = 4 * FONT_X;
	int lineX = x + textWidth - 1; //to separate from border

	// Draw background
	myCanvas->tft->fillRect(x,y,yScaleWidth-borderWidth+1,h-xScaleHeight+borderWidth,bgColor);
	
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

void Trend::drawXScale(){
	unsigned int val;
	int textWidth = 4 * FONT_SPACE;
	
	//Draw X Scale
	//--background
	int xPos = x + yScaleWidth + 1;
	int yPos = y + h + borderWidth - xScaleHeight;
	int xWidth = w - yScaleWidth;
	int xHeight = xScaleHeight;
	myCanvas->tft->fillRect(xPos-yScaleWidth-1,yPos,w,xHeight+1,bgColor);
	yPos += borderWidth;

	//--right
	setNum(0);
	myCanvas->tft->drawString(buf,xPos+xWidth-FONT_X,yPos+10+FONT_Y,1,borderColor);

	xWidth -= borderWidth;
	int xp = xPos + xWidth;
	int longInterval = xWidth / 10;
	
	//for(int i = 0; i < maxValues+1; i++){
		//xp = xPos + xWidth - i*xWidth/(maxValues);	
	for(int i = trendWindow.minValue; i < trendWindow.maxValue+1+1; i++){
		xp = xPos + xWidth - i*xWidth/(trendWindow.maxValue-trendWindow.minValue+1);	
		//draw alternating numbers
		#if !defined(__STM32F1__)
		if(i>0 && !(i%2)){
		#else
		//if(i>0 && !(i%(10))){
		if(i>0 && !(i%((int)xWidth/10))){
		#endif

			myCanvas->tft->drawVerticalLine(xp,yPos,10,borderColor);
			//setNum(map(i,0,maxValues,0,maxX));
			setNum(i);
			byte size = getTextLength(buf);
			myCanvas->tft->drawString(buf,xp - size*(FONT_X>>1),yPos+10+FONT_Y,1,borderColor);	
		//}else{
		}else if(i>0 && !(i%(int)(xWidth/15))){
			myCanvas->tft->drawVerticalLine(xp,yPos,5,borderColor);
		}
	}
	
}

void Trend::drawValues(uint16_t color){
	unsigned int val; //= map(currentValue,scaleMin,scaleMax,h-borderWidth,borderWidth);
	int x1, y1, x2, y2;
	
	//-- loop through all values and plot them left to right (starting with values[7])
	//for(int i = maxValues; i!=0; i--){
	for(int i = trendWindow.maxValue+1; i!=trendWindow.minValue; i--){
		int j = i-1;
		
		//-- previous values[j] coordinates 2-previous 1-current
		x2 = x1;
		y2 = y1;
		
		x1 = getXVal(j);
		y1 = getYVal(values[j]);
		
		//if(j<maxValues-1){
		if(j<trendWindow.maxValue){
			//myCanvas->tft->drawLine(x2,y2-1,x1,y1-1,color);
			//myCanvas->tft->drawLine(x2,y2,x1,y1,color);
			//myCanvas->tft->drawLine(x2,y2+1,x1,y1+1,color);			
			if(forceSquareWaveform){
				myCanvas->tft->drawHorizontalLine(x2,y2,x1-x2+1,color);
				myCanvas->tft->drawVerticalLine(x1,y1,y2-y1,color);
			}else{
				myCanvas->tft->drawLine(x2,y2,x1,y1,color);
			}

		}
	}
}

void Trend::drawBorder(){
	int xPos = x + yScaleWidth;
	int yPos = y;// + borderWidth;
	int width = w-yScaleWidth-2*borderWidth;
	int borderW, borderH;
	borderW = w - yScaleWidth;
	borderH = h - xScaleHeight;

	//--border
	for(byte i=borderWidth; i!=0; i--){
		myCanvas->tft->drawRect(xPos,yPos,borderW,borderH,borderColor);
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
		myCanvas->tft->drawHorizontalLine(xPos+1,getYVal(hiLimit),width,!colors?bgColor:hiLimitColor);
	}
	
	myCanvas->tft->drawHorizontalLine(xPos+1,getYVal(setpoint),width,!colors?bgColor:setpointColor);
	
	if(lowLimit > scaleMin){
		myCanvas->tft->drawHorizontalLine(xPos+1,getYVal(lowLimit),width,!colors?bgColor:lowLimitColor);
	}
}

/*
 * Sets the window of data to be shown on the trend
 */
void Trend::setWindow(int min, int max){
	trendWindow.minValue = min;
	trendWindow.maxValue = max;
}

/* 
 * Sets maximum X value in milli-seconds. This value is used to draw the interval values
 * of the x-scale. If the value is too high, it automatically adjusts
 * to minutes.
 */
/*
void Trend::setMaxX(int m){
	int interval = m/maxValues;
	if(interval*2 >= 60){ //-- auto adjust for minutes interval if too many seconds.
		maxX = m/60;
	}else{
		maxX = m;
	}
}
*/

int Trend::getXVal(int index){
	int xBase = x+yScaleWidth+borderWidth;
	int effWidth = w - yScaleWidth - 2*borderWidth-1;
	//int inc = index*(effWidth)/(maxValues-1);
	int inc = index*(effWidth)/(trendWindow.maxValue-trendWindow.minValue);
	return xBase + effWidth - inc;
}

int Trend::getYVal(int value){
	int yBase = y ;//+ borderWidth ;//+ 1; 	// +1 to account for trend width line
	int maxHeight = h-xScaleHeight-borderWidth - 2; // -2 to account for trend line width
	unsigned int val = constrain(value,scaleMin,scaleMax);
	val = map(val,scaleMin,scaleMax,maxHeight,borderWidth+1); //-1 to account for trend line width
	return yBase + val;
}

int Trend::getMin(){
	int val = values[0];
	for(int i = 1; i<maxValues; i++){
		if(values[i] < val) val = values[i];
	}
	return val;
}

int Trend::getMax(){
	int val = values[0];
	for(int i = 1; i<maxValues; i++){
		if(values[i] > val) val = values[i];
	}
	return val;
}

void Trend::drawTrend(){
	this->drawThresholdLines(true);	
  this->drawValues(this->fgColor);	
}

void Trend::eraseTrend(){
	this->drawValues(this->bgColor);	//erase original line
	this->drawThresholdLines(false); //erase threshold lines		
}

void Trend::autoFit(bool scale){
	eraseTrend();
	
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
	
	drawTrend();	
}

// Adds a new value to the trend
void Trend::addValue(uint8_t val, bool updateTrend){
	previousValue = currentValue;
	currentValue = val;

	//--delete previous line (bgColor)
	if(visible) drawValues(this->bgColor);

	//--push value into the array
	for(int i = maxValues; i!=0; i--){

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
	
	if(updateTrend) update();
}

//Overriden virtual methods
void Trend::show(){
	//Serial.print("Show");Serial.println();
	if(this->visible){
	  myCanvas->tft->fillRect(x,y,w,h,bgColor);
	  this->drawYScale();
	  this->drawXScale();
	  this->drawBorder();
	  this->update();
	}
}

void Trend::update(){
	if(!forcedUpdate){	
		if(!visible) return;
	}
	
	if(enableAutoFit){
		if(values[0] >= scaleMax - 2 || values[0] <= scaleMin + 2){
			autoFit(true);
			return;
		}
		/*
		 *if(updates++ > maxValues){
			autoFit();
			updates = 0;
			return;
		}*/
	}
	
	this->drawThresholdLines(true);
	this->drawValues(this->fgColor);	
}
