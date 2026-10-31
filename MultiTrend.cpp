#include "MultiTrend.h"

MultiTrend::MultiTrend(unsigned int width, unsigned int height,
                       unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit,
                       uint8_t numSeries, uint8_t max_trend_values, uint8_t maxSeries)
{	
	if(maxSeries > MAX_MULTITREND_SERIES) maxSeries = MAX_MULTITREND_SERIES;
	if(numSeries > maxSeries) numSeries = maxSeries;
	this->maxSeries   = maxSeries;
	this->numSeries   = numSeries;
	this->setSize(width,height);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->setColors(BLACK,BLUE,WHITE);
	this->maxValues = max_trend_values;
	init();
}

MultiTrend::~MultiTrend(){
	if(values)      free(values);
	if(seriesColor) free(seriesColor);
}

void MultiTrend::init(){
	if(maxSeries > MAX_MULTITREND_SERIES) maxSeries = MAX_MULTITREND_SERIES;
	if(numSeries > maxSeries) numSeries = maxSeries;
	TrendBase::init();
	type = 0x26;

	// One ring buffer per series (series-major layout) + one color slot each.
	free(values);
	free(seriesColor);
	values = maxSeries && maxValues ? (uint8_t *)malloc((uint32_t)maxSeries * maxValues) : nullptr;
	seriesColor = maxSeries ? (int *)malloc(maxSeries * sizeof(int)) : nullptr;
	if(values){
		memset(values,0,(uint32_t)maxSeries * maxValues * sizeof(uint8_t));
	}
	if(seriesColor){
		// default palette (like the panelboard's CPU/GPU/NPU)
		for(uint8_t s = 0; s < maxSeries; s++){
			seriesColor[s] = (s==0)?BLUE:(s==1)?GREEN:RED;
		}
	}
	for(uint8_t s = 0; s < maxSeries; s++) markerPrevY[s] = -1;
	
	this->hiLimit = scaleMax;
	this->lowLimit = scaleMin;
	this->currentValue = setpoint;
	this->setWindow(0,maxValues-1);
}

void MultiTrend::setSeriesColor(uint8_t series, int color){
	if(seriesColor && series < maxSeries) seriesColor[series] = color;
}

void MultiTrend::setMarkerRadius(uint8_t radius){
	markerRadius = radius;
	if(!radius){
		rightInset = topInset = bottomInset = 0;
		return;
	}
	// Pull the rightmost sample in by radius (+1 clearance) AND give the same
	// clearance above/below (values hit 0 and 100%), so the puck's outer edge
	// never overlaps any border and moving it can't tear the frame.
	rightInset  = radius + 1;
	topInset    = radius + 1;
	bottomInset = radius + 1;
}

void MultiTrend::clear(){
	if(values){
		memset(values,0,(uint32_t)maxSeries * maxValues * sizeof(uint8_t));
	}
}

void MultiTrend::addValue(uint8_t series, uint8_t val, bool updateTrend){
	if(!values || !maxValues || series >= maxSeries) return;
	currentValue = val;
	uint8_t* rowp = values + (uint32_t)series*maxValues;

	if(visible) drawSeries(series, this->bgColor);   // erase old line for this series

	for(int i = maxValues; i!=0; i--){
		if(i==1){
			rowp[i-1] = val;
		}else{
			rowp[i-1] = rowp[i-2];
		}
	}
	if(updateTrend) update();
}

void MultiTrend::drawSeries(uint8_t series, uint16_t color){
	if(!values) return;
	drawValuesSeries(row(series), color);
}

// Erase the previous puck of every series (bg circle at the recorded Y).
// An old puck is a 2·markerRadius blob that sticks out beyond the 1px line,
// so erasing the line alone leaves artifacts — erase the puck explicitly.
// Must run AFTER the line erasure and BEFORE redrawing lines.
void MultiTrend::eraseMarkers(){
	if(!canDraw() || !buf) return;
	if(!markerRadius) return;
	int lx = latestX();
	for(uint8_t s = 0; s < numSeries && s < maxSeries && s < MAX_MULTITREND_SERIES; s++){
		if(markerPrevY[s] >= 0){
			myCanvas->tft->fillCircle(lx, markerPrevY[s], markerRadius+1, this->bgColor);
			markerPrevY[s] = -1;
		}
	}
}

// Draw a puck at each series' newest sample (rightmost), in the series color
// (or severity-tinted). Records the Y used so eraseMarkers() can clear it.
void MultiTrend::drawMarkers(){
	if(!canDraw() || !buf) return;
	if(!markerRadius) return;
	if(!values) return;
	int lx = latestX();
	for(uint8_t s = 0; s < numSeries && s < maxSeries && s < MAX_MULTITREND_SERIES; s++){
		uint8_t v = latest(s);
		int c = seriesColor ? seriesColor[s] : BLUE;
		if(markerCrit > 0 && v >= markerCrit)       c = markerCritColor;
		else if(markerWarn > 0 && v >= markerWarn)  c = markerWarnColor;
		int ly = latestY(s);
		myCanvas->tft->fillCircle(lx, ly, markerRadius, c);
		markerPrevY[s] = ly;
	}
}

void MultiTrend::drawTrend(){
	this->drawThresholdLines(true);
	for(uint8_t s = 0; s < numSeries && s < maxSeries && s < MAX_MULTITREND_SERIES; s++){
		drawSeries(s, seriesColor?seriesColor[s]:BLUE);
	}
	drawMarkers();
}

void MultiTrend::eraseTrend(){
	for(uint8_t s = 0; s < numSeries && s < maxSeries && s < MAX_MULTITREND_SERIES; s++){
		drawSeries(s, this->bgColor);
	}
	eraseMarkers();   // clear pucks too
	this->drawThresholdLines(false);
}

uint8_t MultiTrend::latest(uint8_t series) const{
	if(!values || !maxValues || series >= maxSeries) return 0;
	return row(series)[0];            // index 0 is the newest sample (rightmost)
}

int MultiTrend::getMin(uint8_t series){
	return minOf(row(series));
}

int MultiTrend::getMax(uint8_t series){
	return maxOf(row(series));
}

void MultiTrend::autoFit(bool scale){
	if(!values || !maxValues) return;
	eraseTrend();
	
	if(scale && numSeries){
		int mn = 255, mx = 0;
		for(uint8_t s = 0; s < numSeries && s < maxSeries; ++s){
			const int low = getMin(s), high = getMax(s);
			if(low < mn) mn = low;
			if(high > mx) mx = high;
		}
		scaleMin = mn > 2 ? mn-2 : 0;
		scaleMax = mx+2;
	}
	
	drawYScale();
	drawTrend();	
}

void MultiTrend::show(){
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

void MultiTrend::update(){
	if(!canDraw() || !buf || !values || !maxValues) return;
	if(!forcedUpdate){	
		if(!visible) return;
	}
	
	if(enableAutoFit){
		for(uint8_t s = 0; s < numSeries && s < maxSeries && s < MAX_MULTITREND_SERIES; s++){
			if(row(s)[0] >= (int64_t)scaleMax - 2 || row(s)[0] <= (int64_t)scaleMin + 2){
				autoFit(true);
				return;
			}
		}
	}
	
	// Artifact-free static-under-dynamic: erase all series, repaint the
	// gridlines they crossed, then draw all series on top.
	for(uint8_t s = 0; s < numSeries && s < maxSeries && s < MAX_MULTITREND_SERIES; s++){
		drawSeries(s, this->bgColor);          // erase old plot for this series
	}
	eraseMarkers();                            // clear moved pucks (after line erase)
	if(showGridlines) drawGridlines();
	this->drawThresholdLines(true);
	for(uint8_t s = 0; s < numSeries && s < maxSeries && s < MAX_MULTITREND_SERIES; s++){
		drawSeries(s, seriesColor?seriesColor[s]:BLUE);
	}
	drawMarkers();
}
