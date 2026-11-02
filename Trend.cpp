#include "Trend.h"
//Constructor
// NOTE: buf is allocated by the Indicator base constructor; do NOT malloc it
// again here (would leak the base's allocation).
Trend::Trend(){
	init();
}

Trend::Trend(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit, uint8_t max_trend_values, HistoryStorage storage)
{	
	this->setSize(width,height);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->setColors(BLACK,BLUE,WHITE);
	this->maxValues = max_trend_values;
	historyStorage=storage;
	init();
}

Trend::~Trend(){ free(values); }

//Methods
void Trend::init(){
	headIndex=0;
	resetRenderCache();
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
	headIndex=0;
	if(values){
		memset(values,0,maxValues*sizeof(uint8_t));
	}
}

void Trend::addValue(uint8_t val, bool updateTrend){
	if(!values || !maxValues) return;
	previousValue = currentValue;
	currentValue = val;


	if(historyStorage==HistoryStorage::Circular){
        headIndex=headIndex ? headIndex-1 : maxValues-1;
        values[headIndex]=val;
    }else{
        for(int i=maxValues-1;i>0;--i) values[i]=values[i-1];
        values[0]=val;
    }
	if(updateTrend) update();
}

void Trend::drawTrend(){
    // Retain every intermediate layer footprint until a coherent commit.
    retainLegacyBounds(values,1,0,&headIndex);
	this->drawThresholdLines(true);	
	this->drawValues(this->fgColor);	
}

void Trend::eraseTrend(){
    eraseCommitted();
    retainLegacyBounds(nullptr,0);
    this->drawThresholdLines(false); // erase current threshold chrome
}

void Trend::autoFit(bool scale){
    fitScale(scale);
    update();
}

void Trend::fitScale(bool scale){
	if(!values || !maxValues) return;
	
	if(scale){
		const int mn = getMin(), mx = getMax();
		scaleMin = mn > 2 ? mn-2 : 0;
		scaleMax = mx+2;
	}

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
    invalidate();
    update();
}

void Trend::update(){
	if(!canDraw() || !buf || !values || !maxValues) return;
	if(!forcedUpdate){	
		if(!visible) return;
	}
	
	if(enableAutoFit){
		if(sample(0) >= (int64_t)scaleMax - 2 || sample(0) <= (int64_t)scaleMin + 2){
			fitScale(true);
		}
	}
	
    const int color = fgColor;
    commitFrame(values,1,&color,0,0,0,0,0,forcedUpdate,&headIndex);
}

void Trend::drawValues(uint16_t color){
	drawValuesSeries(values, color, headIndex);
}

uint8_t Trend::sample(int i) const{
    if(!values || !maxValues || i<0 || i>=maxValues) return 0;
    return values[((unsigned)headIndex+i)%maxValues];
}
