#include "TrendBase.h"
#include <stdio.h>

TrendBase::TrendBase(){
}

TrendBase::~TrendBase(){
}

// Shared: everything drawChart chrome needs. type stays in the concrete
// subclass (reserved 0x26 for MultiTrend), but the base sets Indicator's
// common defaults.
void TrendBase::init(){
	Indicator::init();
	borderWidth = 2;
	this->setWindow(0, maxValues-1);
}

void TrendBase::setWindow(int min, int max){
	const int last = maxValues ? maxValues - 1 : 0;
	min = constrain(min, 0, last);
	max = constrain(max, 0, last);
	if(min > max){ int tmp = min; min = max; max = tmp; }
	trendWindow.minValue = min;
	trendWindow.maxValue = max;
}

void TrendBase::setMaxX(int m){
	maxX = m;
}

int TrendBase::getXVal(int index){
	setWindow(trendWindow.minValue, trendWindow.maxValue);
	if(w <= 0) return x;
	const int margin = constrain(chartX(), 0, w-1);
	const int bw = constrain(borderWidth, 0, w-1-margin);
	const int xBase = x + margin + bw;
	int effWidth = w - margin - 2*bw - 1 - (rightInset > 0 ? rightInset : 0);
	if(effWidth < 0) effWidth = 0;
	const int span = trendWindow.maxValue - trendWindow.minValue;
	if(span <= 0) return xBase + effWidth;
	index = constrain(index, trendWindow.minValue, trendWindow.maxValue);
	const int inc = (int)((int64_t)(index-trendWindow.minValue)*effWidth/span);
	return xBase + effWidth - inc;
}

int TrendBase::getYVal(int value){
	return getYValValue(value);
}

int TrendBase::getYVal(unsigned int value){
	return getYValValue(value);
}

int TrendBase::getYValValue(int64_t value){
	const int last = h > 0 ? h-1 : 0;
	int top = constrain(borderWidth+1+topInset, 0, last);
	int bottom = constrain(chartH()-borderWidth-2-bottomInset, top, last);
	return y + mapScale(value, bottom, top);
}

void TrendBase::drawYScale(){
	if(!canDraw() || !buf) return;
	if(!showYScale || w <= 0 || chartH() <= 0) return;
	// Y SCALE
	int textWidth = 4 * FONT_X;
	int lineX = x + textWidth - 1;

	const int scaleWidth = yScaleWidth-borderWidth+1;
	const int scaleHeight = chartH()+borderWidth;
	if(scaleWidth > 0 && scaleHeight > 0)
		myCanvas->tft->fillRect(x,y,scaleWidth,scaleHeight,bgColor);

	setNum(scaleMax);
	myCanvas->tft->drawString(buf,x,getYVal(scaleMax),1,borderColor);
	myCanvas->tft->drawHorizontalLine(lineX,getYVal(scaleMax),10,borderColor);

	setNum(setpoint);
	myCanvas->tft->drawString(buf,x,getYVal(setpoint)-(FONT_Y/2),1,setpointColor);
	myCanvas->tft->drawHorizontalLine(lineX,getYVal(setpoint),10,setpointColor);

	setNum(scaleMin);
	myCanvas->tft->drawString(buf,x,getYVal(scaleMin)-(FONT_Y),1,borderColor);
	myCanvas->tft->drawHorizontalLine(lineX,getYVal(scaleMin),10,borderColor);

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

void TrendBase::drawXScale(){
	if(!canDraw() || !buf || !showXScale || w <= chartX() || h <= 0 || !maxValues) return;
	setWindow(trendWindow.minValue, trendWindow.maxValue);
	const int span = trendWindow.maxValue-trendWindow.minValue;
	const int yPos = y+chartH()+borderWidth;
	myCanvas->tft->fillRect(x,y+chartH(),w,xScaleHeight,bgColor);
	const int width = w-chartX()-2*borderWidth;
#if defined(__STM32F1__)
	const int majorStep = width >= 10 ? width/10 : 1;
#else
	const int majorStep = 2;
#endif
	const int minorStep = width >= 15 ? width/15 : 1;
	for(int offset = 0; offset <= span; ++offset){
		const int index = trendWindow.minValue+offset;
		const int xp = getXVal(index);
		if(offset == 0 || offset == span || offset%majorStep == 0){
			myCanvas->tft->drawVerticalLine(xp,yPos,10,borderColor);
			if(maxX > 0){
				const int secs = span ? (int)((int64_t)offset*maxX/span) : 0;
				if(secs < 60) setNum(secs);
				else snprintf(buf,BUF_SIZE+1,"%d:%02d",secs/60,secs%60);
			}else setNum(index);
			int tx = xp-getTextLength(buf)*(FONT_X/2);
			if(tx < x) tx = x;
			myCanvas->tft->drawString(buf,tx,yPos+10+FONT_Y,1,borderColor);
		}else if(offset%minorStep == 0){
			myCanvas->tft->drawVerticalLine(xp,yPos,5,borderColor);
		}
	}
}

void TrendBase::drawBorder(){
	if(!canDraw() || !buf) return;
	if(!showBorder) return;
	int xPos = x + chartX();
	int yPos = y;
	int borderW = w - chartX();
	int borderH = chartH();
	for(int i=borderWidth; i>0 && borderW>0 && borderH>0; --i){
		myCanvas->tft->drawRect(xPos,yPos,borderW,borderH,borderColor);
		xPos++;
		yPos++;
		borderW -= 2;
		borderH -= 2;
	}
}

// 25/50/75 % guide lines across the chart (uses the chart's real geometry
// so they line up with plotted data even in bare mode).
void TrendBase::drawGridlines(){
	if(!canDraw() || !buf) return;
	int xl = x + chartX() + borderWidth;
	int width = w - chartX() - 2*borderWidth;
	if(width <= 0 || chartH() <= 0) return;
	uint16_t gc = 0x0841;   // faint grid color
	for(int p = 25; p <= 75; p += 25){
		myCanvas->tft->drawHorizontalLine(xl, getYVal(p), width, gc);
	}
}

void TrendBase::drawThresholdLines(bool colors){
	if(!canDraw() || !buf) return;
	if(!showThresholds) return;
	int xPos = x + borderWidth + chartX() - 1;
	int width = w - chartX() - 2*borderWidth;
	if(width <= 0 || chartH() <= 0) return;

	if(hiLimit < scaleMax){
		myCanvas->tft->drawHorizontalLine(xPos+1,getYVal(hiLimit),width,!colors?bgColor:hiLimitColor);
	}
	myCanvas->tft->drawHorizontalLine(xPos+1,getYVal(setpoint),width,!colors?bgColor:setpointColor);
	if(lowLimit > scaleMin){
		myCanvas->tft->drawHorizontalLine(xPos+1,getYVal(lowLimit),width,!colors?bgColor:lowLimitColor);
	}
}

void TrendBase::drawValuesSeries(const uint8_t* values, uint16_t color){
	if(!canDraw() || !buf || !values || !maxValues || w <= 0 || h <= 0) return;
	// Validate the public window as well as setWindow() callers.
	setWindow(trendWindow.minValue, trendWindow.maxValue);
	const int first = constrain(trendWindow.minValue, 0, (int)maxValues-1);
	const int last = constrain(trendWindow.maxValue, first, (int)maxValues-1);
	int x2 = getXVal(last), y2 = getYVal(values[last]);
	for(int j = last-1; j >= first; --j){
		const int x1 = getXVal(j), y1 = getYVal(values[j]);
		if(forceSquareWaveform){
			const int left = x1 < x2 ? x1 : x2;
			const int top = y1 < y2 ? y1 : y2;
			myCanvas->tft->drawHorizontalLine(left,y2,abs(x1-x2)+1,color);
			myCanvas->tft->drawVerticalLine(x1,top,abs(y2-y1)+1,color);
		}else{
			myCanvas->tft->drawLine(x2,y2,x1,y1,color);
		}
		x2 = x1; y2 = y1;
	}
}

int TrendBase::minOf(const uint8_t* values){
	if(!values || !maxValues) return scaleMin;
	int val = values[0];
	for(int i = 1; i<maxValues; i++) if(values[i] < val) val = values[i];
	return val;
}

int TrendBase::maxOf(const uint8_t* values){
	if(!values || !maxValues) return scaleMax;
	int val = values[0];
	for(int i = 1; i<maxValues; i++) if(values[i] > val) val = values[i];
	return val;
}
