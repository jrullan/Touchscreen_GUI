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

void Trend::drawScale(){
	unsigned int val;
	//Draw SCALE
	int textWidth = 4 * FONT_SPACE;
	
	// Draw background
	//Tft.fillRectangle(x+textWidth,y+borderWidth,10,h,BLACK);
	Tft.fillRectangle(x+borderWidth,y+borderWidth,yScaleWidth,h-xScaleHeight,BLACK);
	
	// Draw Max value and line
	setNum(scaleMax); // Sets buf with scaleMax value
	Tft.drawString(buf,x,y,1,borderColor);
	Tft.drawHorizontalLine(x+textWidth,y+borderWidth,10,borderColor);
	
	// Draw Setpoint value and line
	setNum(setpoint); // Sets buf with setpoint value
	val = map(setpoint,scaleMin,scaleMax,h-borderWidth-xScaleHeight,borderWidth);
	Tft.drawString(buf,x,y+(val)-(FONT_Y>>1)-borderWidth,1,fgColor);
	Tft.drawHorizontalLine(x+textWidth,getYVal(setpoint),10,fgColor);
	
	// Draw Min value and line
	setNum(scaleMin); // Sets buf with scaleMin value
	Tft.drawString(buf,x,y+h-xScaleHeight-(FONT_Y),1,borderColor);
	Tft.drawHorizontalLine(x+textWidth,y+(h-xScaleHeight)-borderWidth,10,borderColor);

	// Draw HI and LOW limits values and lines
  if(hiLimit < scaleMax){
	  setNum(hiLimit);
	  Tft.drawString(buf,x,getYVal(hiLimit)-(FONT_Y/2),1,hiLimitColor);
		Tft.drawHorizontalLine(x+textWidth,getYVal(hiLimit),10,hiLimitColor);
  }
  if(lowLimit > scaleMin){
	  setNum(lowLimit);
	  Tft.drawString(buf,x,getYVal(lowLimit)-(FONT_Y/2),1,lowLimitColor);
		Tft.drawHorizontalLine(x+textWidth,getYVal(lowLimit),10,lowLimitColor);
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
		
		x1 = getXVal(values[j],j);
		y1 = getYVal(values[j]);
		
		if(j<MAX_TREND_VALUES-1){
			Tft.drawLine(x2,y2-1,x1,y1-1,color);
			Tft.drawLine(x2,y2,x1,y1,color);
			Tft.drawLine(x2,y2+1,x1,y1+1,color);
		}
	}
}

int Trend::getXVal(int value,int index){
	//int scaleWidth = 4 * FONT_SPACE + 10;
	int xBase = x+borderWidth+yScaleWidth+1;
	int effWidth = w - yScaleWidth - 2*borderWidth;
	int inc = index*(effWidth)/(MAX_TREND_VALUES-1);
	return xBase + effWidth - inc;
}

int Trend::getYVal(int value){
	int yBase = y + borderWidth;
	unsigned int val = constrain(value,scaleMin,scaleMax);
	val = map(val,scaleMin,scaleMax,h-xScaleHeight-borderWidth,borderWidth);
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

void Trend::autoFit(){
	int min = getMin();
	int max = getMax();
	
	//Serial.print("Min = ");
	//Serial.println(min);
	//Serial.print("Max = ");
	//Serial.println(max);
	
	if(min <= scaleMin && min != 0){ 
		scaleMin = min-5;
	}else{
		if(min-scaleMin > 5) scaleMin = min - 5;
	}
	
	if(max >= scaleMax){
		scaleMax = max+5;
	}else{
		if(scaleMax-max > 5) scaleMax = max + 5;
	}
	//hide();
	//show();
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
  this->drawScale();
  this->update();
}

void Trend::update(){
	if(!forcedUpdate){	
		if(!visible) return;
	}
	
	if(enableAutoFit){
		if(updates++ > MAX_TREND_VALUES){
			hide();
			autoFit();
			drawScale();
			updates = 0;
		}
	}
	
	//int scaleWidth = 4 * FONT_SPACE + 10;
	int xPos = x + borderWidth + yScaleWidth - 1;
	int width = w-yScaleWidth-2*borderWidth;
  int yPos = y + borderWidth;
  int height = h;//-yPos;
  
  int borderX, borderY, borderW, borderH;
  borderX = xPos;
  borderY = yPos;
  borderW = w-yScaleWidth;
  borderH = height - xScaleHeight;
  
  //--border
  for(byte i=borderWidth; i!=0; i--){
    Tft.drawRectangle(borderX,borderY,borderW,borderH,borderColor);
    borderX++;
    borderY++;
    borderW -= 2;
    borderH -= 2;
  }
  
	//--background fill
	//Tft.fillRectangle(x+borderWidth+scaleWidth, y+borderWidth, w-(2*borderWidth), h-(2*borderWidth),bgColor);
	
	//--threshold lines
	if(hiLimit < scaleMax){
		Tft.drawHorizontalLine(xPos+1,getYVal(hiLimit),width,hiLimitColor);
	}
	Tft.drawHorizontalLine(xPos+1,getYVal(setpoint),width,this->borderColor);
	if(lowLimit > scaleMin){
		Tft.drawHorizontalLine(xPos+1,getYVal(lowLimit),width,lowLimitColor);
	}

  //-- drawValues
  this->drawValues(this->fgColor);
}
