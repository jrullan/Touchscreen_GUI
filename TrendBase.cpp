#include "TrendBase.h"

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
	trendWindow.minValue = min;
	trendWindow.maxValue = max;
}

void TrendBase::setMaxX(int m){
	maxX = m;
}

int TrendBase::getXVal(int index){
	int xBase = x + chartX() + borderWidth;
	int effWidth = w - chartX() - 2*borderWidth - 1 - rightInset;
	int inc = index*(effWidth)/(trendWindow.maxValue-trendWindow.minValue);
	return xBase + effWidth - inc;
}

int TrendBase::getYVal(int value){
	int yBase = y;
	int topH = showXScale ? xScaleHeight : 0;
	int maxHeight = h - topH - borderWidth - 2 - bottomInset;
	unsigned int val = constrain(value, scaleMin, scaleMax);
	val = map(val, scaleMin, scaleMax, maxHeight, borderWidth+1+topInset);
	return yBase + val;
}

void TrendBase::drawYScale(){
	if(!showYScale) return;
	// Y SCALE
	int textWidth = 4 * FONT_X;
	int lineX = x + textWidth - 1;

	myCanvas->tft->fillRect(x,y,yScaleWidth-borderWidth+1,h-xScaleHeight+borderWidth,bgColor);

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
	if(!showXScale) return;
	unsigned int val;
	int textWidth = 4 * FONT_SPACE;

	// X scale background
	int xPos = x + yScaleWidth + 1;
	int yPos = y + h + borderWidth - xScaleHeight;
	int xWidth = w - yScaleWidth;
	int xHeight = xScaleHeight;
	myCanvas->tft->fillRect(xPos-yScaleWidth-1,yPos,w,xHeight+1,bgColor);
	yPos += borderWidth;

	// right label ("0")
	setNum(0);
	myCanvas->tft->drawString(buf,xPos+xWidth-FONT_X,yPos+10+FONT_Y,1,borderColor);

	xWidth -= borderWidth;
	int xp = xPos + xWidth;

	const int span = trendWindow.maxValue - trendWindow.minValue + 1;
	for(int i = trendWindow.minValue; i < trendWindow.maxValue+1+1; i++){
		xp = xPos + xWidth - i*xWidth/span;
		#if !defined(__STM32F1__)
		if(i>0 && !(i%2)){
		#else
		if(i>0 && !(i%((int)xWidth/10))){
		#endif
			myCanvas->tft->drawVerticalLine(xp,yPos,10,borderColor);
			if(maxX > 0){
				int secs = (int)((long)(i - trendWindow.minValue) * maxX / span);
				if(secs < 60){
					setNum(secs);
				}else{
					int m = secs/60, s = secs%60;
					buf[0]='0'+m; buf[1]=':';
					buf[2]='0'+(s/10); buf[3]='0'+(s%10); buf[4]=0;
				}
			}else{
				setNum(i);
			}
			byte size = getTextLength(buf);
			myCanvas->tft->drawString(buf,xp - size*(FONT_X>>1),yPos+10+FONT_Y,1,borderColor);
		}else if(i>0 && !(i%(int)(xWidth/15))){
			myCanvas->tft->drawVerticalLine(xp,yPos,5,borderColor);
		}
	}
}

void TrendBase::drawBorder(){
	if(!showBorder) return;
	int xPos = x + chartX();
	int yPos = y;
	int borderW = w - chartX();
	int borderH = chartH();
	for(byte i=borderWidth; i!=0; i--){
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
	int xl = x + chartX() + borderWidth;
	int width = w - chartX() - 2*borderWidth;
	uint16_t gc = 0x0841;   // faint grid color
	for(int p = 25; p <= 75; p += 25){
		myCanvas->tft->drawHorizontalLine(xl, getYVal(p), width, gc);
	}
}

void TrendBase::drawThresholdLines(bool colors){
	if(!showThresholds) return;
	int xPos = x + borderWidth + chartX() - 1;
	int width = w - chartX() - 2*borderWidth;

	if(hiLimit < scaleMax){
		myCanvas->tft->drawHorizontalLine(xPos+1,getYVal(hiLimit),width,!colors?bgColor:hiLimitColor);
	}
	myCanvas->tft->drawHorizontalLine(xPos+1,getYVal(setpoint),width,!colors?bgColor:setpointColor);
	if(lowLimit > scaleMin){
		myCanvas->tft->drawHorizontalLine(xPos+1,getYVal(lowLimit),width,!colors?bgColor:lowLimitColor);
	}
}

void TrendBase::drawValuesSeries(const uint8_t* values, uint16_t color){
	if(!values) return;
	int x1, y1, x2, y2;
	for(int i = trendWindow.maxValue+1; i != trendWindow.minValue; i--){
		int j = i-1;
		x2 = x1; y2 = y1;
		x1 = getXVal(j);
		y1 = getYVal(values[j]);
		if(j < trendWindow.maxValue){
			if(forceSquareWaveform){
				myCanvas->tft->drawHorizontalLine(x2,y2,x1-x2+1,color);
				myCanvas->tft->drawVerticalLine(x1,y1,y2-y1,color);
			}else{
				myCanvas->tft->drawLine(x2,y2,x1,y1,color);
			}
		}
	}
}

int TrendBase::minOf(const uint8_t* values){
	if(!values) return scaleMin;
	int val = values[0];
	for(int i = 1; i<maxValues; i++) if(values[i] < val) val = values[i];
	return val;
}

int TrendBase::maxOf(const uint8_t* values){
	if(!values) return scaleMax;
	int val = values[0];
	for(int i = 1; i<maxValues; i++) if(values[i] > val) val = values[i];
	return val;
}
