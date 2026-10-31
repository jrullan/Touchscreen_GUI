#include "Trend.h"
//Constructor
// NOTE: buf is allocated by the Indicator base constructor; do NOT malloc it
// again here (would leak the base's allocation).
Trend::Trend(){
}

Trend::Trend(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit, uint8_t max_trend_values)
{	
	this->setSize(width,height);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->setColors(BLACK,BLUE,WHITE);
	this->maxValues = max_trend_values;
	init();
}

Trend::~Trend(){}

//Methods
void Trend::init(){
	TrendBase::init();
	type = 0x23;

	// Reserve memory for trend values (values is NULL on failure; every
	// consumer checks it before dereferencing).
	values = (uint8_t *)malloc(maxValues);
	if(values){
		memset(values,0,maxValues*sizeof(uint8_t));
	}
	
	this->hiLimit = scaleMax;
	this->lowLimit = scaleMin;
	this->currentValue = setpoint;
	this->setWindow(0,maxValues-1);
}

// Clears the TREND VALUES (the actual data plotted), not the scratch label
// buffer. Indicator::setNum() calls Indicator::clear() non-virtually, so
// label rendering is unaffected.
void Trend::clear(){
	if(values){
		memset(values,0,maxValues*sizeof(uint8_t));
	}
}

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
	}
	
	if(updateTrend) update();
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

int Trend::getMin(){
	return minOf(values);
}

int Trend::getMax(){
	return maxOf(values);
}

//Overriden virtual methods
void Trend::show(){
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
	}
	
	this->drawThresholdLines(true);
	this->drawValues(this->fgColor);	
}

void Trend::drawValues(uint16_t color){
	drawValuesSeries(values, color);
}
