#include "Trend.h"
//Constructor
// NOTE: buf is allocated by the Indicator base constructor; do NOT malloc it
// again here (would leak the base's allocation).
Trend::Trend(){
	init();
}

Trend::Trend(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit, uint8_t max_trend_values)
{	
	this->setSize(width,height);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->setColors(BLACK,BLUE,WHITE);
	this->maxValues = max_trend_values;
	init();
}

Trend::~Trend(){ free(values); }

//Methods
void Trend::init(){
	TrendBase::init();
	type = 0x23;

	// Reserve memory for trend values (values is NULL on failure; every
	// consumer checks it before dereferencing).
	free(values);
	values = maxValues ? (uint8_t *)malloc(maxValues) : nullptr;
	vals = 0;
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
	if(!values || !maxValues) return;
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
	if(!values || !maxValues) return;
	eraseTrend();
	
	if(scale){
		const int mn = getMin(), mx = getMax();
		scaleMin = mn > 2 ? mn-2 : 0;
		scaleMax = mx+2;
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
	visible = true;
	if(!canDraw() || !buf) return;
	if(this->visible){
	  myCanvas->tft->fillRect(x,y,w,h,bgColor);
	  this->drawYScale();
	  this->drawXScale();
	  this->drawBorder();
	  this->update();
	}
}

void Trend::update(){
	if(!canDraw() || !buf || !values || !maxValues) return;
	if(!forcedUpdate){	
		if(!visible) return;
	}
	
	if(enableAutoFit){
		if(values[0] >= (int64_t)scaleMax - 2 || values[0] <= (int64_t)scaleMin + 2){
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
