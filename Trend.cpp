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
	Tft.drawString(buf,x+FONT_SPACE,y+(val)-(FONT_Y>>1)-borderWidth,1,fgColor);
	Tft.drawHorizontalLine(x+textWidth,y+(val)-borderWidth,10,fgColor);
	
	// Draw Min value and line
	setNum(scaleMin); // Sets buf with scaleMin value
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

void Trend::drawValues(){
	unsigned int val; //= map(currentValue,scaleMin,scaleMax,h-borderWidth,borderWidth);
	int textWidth = 4 * FONT_SPACE + 10;
	byte xBase = x+borderWidth+textWidth;
	byte yBase = y;
	byte x1, y1, x2, y2;
	byte radius = 2;
	
	//-- loop through all values and plot them left to right (starting with values[7])
	for(int i = MAX_TREND_VALUES; i!=0; i--){
		int j = i-1;
		val = constrain(values[j],scaleMin,scaleMax);
		val = map(val,scaleMin,scaleMax,h-borderWidth,borderWidth);
		yBase = y + val;
		
		//-- previous values[j] coordinates
		x2 = x1;
		y2 = y1;
		
		//-- current values[j] coordinates
		byte effWidth = w - 2*borderWidth;
		byte inc = j*(effWidth)/(MAX_TREND_VALUES-1);
		x1 = xBase + effWidth - inc;
		y1 = yBase;
		
		if(j<MAX_TREND_VALUES-1) Tft.drawLine(x2,y2,x1,y1,this->fgColor);
		Tft.fillCircle(x1, y1, radius, this->fgColor);
	}
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
		//if(previousValue == currentValue) return;
	}
	
	int textWidth = 4 * FONT_SPACE + 10;
	int xPos = x + textWidth;
	int width = w;
  byte yPos = y;
  byte height = h;
  unsigned int val;
  
  //Serial.println("Updating");
	//Serial.print("Millis ");
	//Serial.print(millis());  
  //Serial.print("x = ");
  //Serial.print(x);
  //Serial.print(" y = ");
  //Serial.println(y);
  
	//Draw the Vertical Bar
	
  //--border
  for(byte i=borderWidth; i!=0;i--){
    Tft.drawRectangle(xPos++,yPos++,width--,height--,borderColor);
    width--;
    height--;
  }
  val = map(currentValue,scaleMin,scaleMax,h-borderWidth,borderWidth);
  
	//--background fill
	Tft.fillRectangle(x+borderWidth+textWidth, y+borderWidth, w-(2*borderWidth), h-(2*borderWidth),bgColor);
	
	//--bar fill
	//int color=fgColor;
	//if(currentValue > this->hiLimit) color = hiLimitColor;
	//if(currentValue < this->lowLimit) color = lowLimitColor;
  //Tft.fillRectangle(x+borderWidth+textWidth, y+val, w-(2*borderWidth),h-val-borderWidth,color);
  
  //-- drawValues
  this->drawValues();
}
