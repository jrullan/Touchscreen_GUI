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
	Tft.fillRectangle(x+textWidth,y+borderWidth,10,h,BLACK);
	
	// Draw Max value and line
	setNum(scaleMax); // Sets buf with scaleMax value
	Tft.drawString(buf,x,y,1,borderColor);
	Tft.drawHorizontalLine(x+textWidth,y+borderWidth,10,borderColor);
	
	// Draw Setpoint value and line
	setNum(setpoint); // Sets buf with setpoint value
	val = map(setpoint,scaleMin,scaleMax,h-borderWidth,borderWidth);
	Tft.drawString(buf,x,y+(val)-(FONT_Y>>1)-borderWidth,1,fgColor);
	//Tft.drawHorizontalLine(x+textWidth,y+(val)-borderWidth,10,fgColor);
	Tft.drawHorizontalLine(x+textWidth,getYVal(setpoint),10,fgColor);
	
	// Draw Min value and line
	setNum(scaleMin); // Sets buf with scaleMin value
	Tft.drawString(buf,x,y+h-(FONT_Y),1,borderColor);
	Tft.drawHorizontalLine(x+textWidth,y+(h)-borderWidth,10,borderColor);

	
  if(hiLimit < scaleMax){// && currentValue > lowLimit){
	  //val = map(hiLimit,scaleMin,scaleMax,h-borderWidth,borderWidth);
	  setNum(hiLimit);
	  Tft.drawString(buf,x,getYVal(hiLimit)-(FONT_Y/2),1,hiLimitColor);
		Tft.drawHorizontalLine(x+textWidth,getYVal(hiLimit),10,hiLimitColor);
		//Tft.drawHorizontalLine(x+textWidth,y+val,10,hiLimitColor);
  	//Tft.drawHorizontalLine(x+borderWidth+textWidth,y+val,w-(2*borderWidth),hiLimitColor);
  }
  if(lowLimit > scaleMin){// && currentValue < hiLimit){
	  //val = map(lowLimit,scaleMin,scaleMax,h-borderWidth,borderWidth);
	  setNum(lowLimit);
	  Tft.drawString(buf,x,getYVal(lowLimit)-(FONT_Y/2),1,lowLimitColor);
		Tft.drawHorizontalLine(x+textWidth,getYVal(lowLimit),10,lowLimitColor);
		//Tft.drawHorizontalLine(x+textWidth,y+val,10,lowLimitColor);
  	//Tft.drawHorizontalLine(x+borderWidth+textWidth,y+val,w-(2*borderWidth),lowLimitColor);
  }
	//drawLimits();
}

void Trend::drawValues(){
	unsigned int val; //= map(currentValue,scaleMin,scaleMax,h-borderWidth,borderWidth);
	int textWidth = 4 * FONT_SPACE + 10;
	int xBase = x+borderWidth+textWidth;
	int yBase = y;
	int x1, y1, x2, y2;
	int radius = 1;
	
	//-- loop through all values and plot them left to right (starting with values[7])
	for(int i = MAX_TREND_VALUES; i!=0; i--){
		int j = i-1;
		//val = constrain(values[j],scaleMin,scaleMax);
		//val = map(val,scaleMin,scaleMax,h-borderWidth,borderWidth);
		//yBase = y + val;
		
		//-- previous values[j] coordinates 2-previous 1-current
		x2 = x1;
		y2 = y1;
		
		//-- current values[j] coordinates
		//int effWidth = w - 2*borderWidth;
		//int inc = j*(effWidth)/(MAX_TREND_VALUES-1);
		//x1 = xBase + effWidth - inc;
		//y1 = yBase;
		
		
		x1 = getXVal(values[j],j);
		y1 = getYVal(values[j]);
		
		
		if(j<MAX_TREND_VALUES-1){
			//unsigned int x3,y3,yp;
			//yp = getYVal(values[j+1],j+1);
			//x3 = x2+(x1-x2)/3;
			//y3 = y2+(y1-y2)/3;		

			//if(yp<y2 && y2>y1){ //-- up slope
			//	y3 = y2 - (y2-y1)/4;
			//}
			//if(yp>y2 && y2<y1){
			//	y3 = y2 + (y1-y2)/4;
			//}
			
			//Serial.print("y3 = ");
			//Serial.println(y3);
			
			//Tft.drawLine(x2,y2,x3,y3,this->fgColor);
			//Tft.drawLine(x3,y3,x1,y1,this->fgColor);
			Tft.drawLine(x2,y2-1,x1,y1-1,this->fgColor);
			Tft.drawLine(x2,y2,x1,y1,this->fgColor);
			Tft.drawLine(x2,y2+1,x1,y1+1,this->fgColor);
		}
		
		//Tft.fillCircle(x1, y1, radius, this->fgColor);
	}
}

int Trend::getXVal(int value,int index){
	int textWidth = 4 * FONT_SPACE + 10;
	int xBase = x+borderWidth+textWidth;
	int effWidth = w - 2*borderWidth;
	int inc = index*(effWidth)/(MAX_TREND_VALUES-1);
	return xBase + effWidth - inc;
}

int Trend::getYVal(int value){
	int yBase = y;
	unsigned int val = constrain(value,scaleMin,scaleMax);
	val = map(val,scaleMin,scaleMax,h-borderWidth,borderWidth);
	return y + val;
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
	
	Serial.print("Min = ");
	Serial.println(min);
	Serial.print("Max = ");
	Serial.println(max);
	
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

	//-- push value into the array
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
	
	int textWidth = 4 * FONT_SPACE + 10;
	int xPos = x + textWidth;
	int width = w;
  byte yPos = y;
  byte height = h;
  //unsigned int val;
  
  //--border
  for(byte i=borderWidth; i!=0;i--){
    Tft.drawRectangle(xPos++,yPos++,width--,height--,borderColor);
    width--;
    height--;
  }
  //val = map(currentValue,scaleMin,scaleMax,h-borderWidth,borderWidth);
  
	//--background fill
	Tft.fillRectangle(x+borderWidth+textWidth, y+borderWidth, w-(2*borderWidth), h-(2*borderWidth),bgColor);
	
	//--threshold lines
	if(hiLimit < scaleMax){
		Tft.drawHorizontalLine(x+borderWidth+textWidth,getYVal(hiLimit),w-2*borderWidth,hiLimitColor);
	}
	Tft.drawHorizontalLine(x+borderWidth+textWidth,getYVal(setpoint),w-2*borderWidth,this->borderColor);
	if(lowLimit > scaleMin){
		Tft.drawHorizontalLine(x+borderWidth+textWidth,getYVal(lowLimit),w-2*borderWidth,lowLimitColor);
	}

  //-- drawValues
  this->drawValues();
}
