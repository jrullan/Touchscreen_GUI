#include "Indicator.h"
#include <stdio.h>

Indicator::Indicator(){
	if( (buf = (char *)malloc(BUF_SIZE+1)) ) memset(buf,0,BUF_SIZE+1); //Had to add one more, to avoid some bug
}

Indicator::~Indicator(){
	free(buf);
}

void Indicator::clear(){
	if(buf) memset(buf, 0, BUF_SIZE+1);
}

unsigned int Indicator::getCV(){
	return currentValue;
}

void Indicator::init(){
	// Constructor leaves buf null on OOM; retry without replacing live storage.
	if(!buf){
		buf = (char *)malloc(BUF_SIZE+1);
		if(buf) memset(buf, 0, BUF_SIZE+1);
	}
	type = 0x20;
	x=0;
	y=0;
	borderWidth=3;
}

void Indicator::setCV(int cv, boolean show){
	setCVValue(cv, show);
}

void Indicator::setCV(unsigned int cv, boolean show){
	setCVValue(cv, show);
}

void Indicator::setCVValue(int64_t cv, boolean show){
	previousValue = currentValue;
	currentValue = cv < 0 || (uint64_t)cv < scaleMin ? scaleMin :
		((uint64_t)cv > scaleMax ? scaleMax : (unsigned int)cv);
	if(show){
		update();
	}
}

void Indicator::setHiLimit(unsigned int h, int color){
	this->hiLimit = h;
	this->hiLimitColor = color;
}

void Indicator::setLimits(unsigned int min, unsigned int sp, unsigned int max){
	if(min > max){ unsigned int tmp = min; min = max; max = tmp; }
	this->scaleMax = max;
	this->scaleMin = min;
	this->setpoint = constrain(sp, min, max);
}

void Indicator::setLowLimit(unsigned int l, int color){
	this->lowLimit = l;
	this->lowLimitColor = color;
}

void Indicator::setSP(unsigned int sp, int color){
	this->setpoint = sp;
	this->setpointColor = color;
}

int Indicator::mapScale(int64_t value, int bottom, int top) const{
	int64_t lo = scaleMin, hi = scaleMax;
	if(hi <= lo) return bottom;
	if(value < lo) value = lo;
	if(value > hi) value = hi;
	return bottom + (int)((value-lo)*((int64_t)top-bottom)/(hi-lo));
}

void Indicator::setNum(int num){
	if(buf) snprintf(buf, BUF_SIZE+1, "%d", num);
}

void Indicator::setNum(unsigned int num){
	if(buf) snprintf(buf, BUF_SIZE+1, "%u", num);
}

//Overriden Methods

bool Indicator::checkTouch(Point* p){
	return true;
}

/*
void Indicator::show(){
	update();
}

void Indicator::update(){
	
}
*/
