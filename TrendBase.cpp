#include "TrendBase.h"
#include <stdio.h>

namespace {
// Isolate classic 6x8 glyph state from the caller's font/cursor/style. This
// lightweight stack object owns no pixels and never changes the shared TFT.
// Only the common GFX drawPixel API is required (also by the STM32 AS backend).
class XAxisGlyphs : public Adafruit_GFX {
public:
    explicit XAxisGlyphs(Guitft& tft)
        : Adafruit_GFX(tft.width(), tft.height()), tft_(tft) {}
    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        tft_.drawPixel(x, y, color);
    }
private:
    Guitft& tft_;
};
}

TrendBase::TrendBase(){
}

TrendBase::~TrendBase(){
	free(rendered);
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

int TrendBase::xScaleLabel(int offset, int& textX){
	const int span = trendWindow.maxValue-trendWindow.minValue;
	if(maxX > 0){
		// maxX is seconds, not a millis() timestamp. Widen before multiplying.
		const int secs = span ? (int)((int64_t)offset*maxX/span) : 0;
		if(secs < 60) setNum(secs);
		else snprintf(buf,BUF_SIZE+1,"%d:%02d",secs/60,secs%60);
	}else setNum(trendWindow.minValue+offset);
	// XAxisGlyphs renders size-one classic cells irrespective of the caller's
	// GFX font or Widget::fontSize. Do not narrow text widths to a byte.
	const int width = (int)strlen(buf)*FONT_X;
	if(width > w) return 0;
	textX = constrain(getXVal(trendWindow.minValue+offset)-width/2, x, x+w-width);
	return width;
}

void TrendBase::drawXScale(){
	if(!canDraw() || !buf || !showXScale || w <= chartX() || h <= 0 || !maxValues) return;
	setWindow(trendWindow.minValue, trendWindow.maxValue);
	const int height = chartH();
	// No allocated scale strip: never send negative positions to uint16 APIs.
	if(height < 0) return;
	const int span = trendWindow.maxValue-trendWindow.minValue;
	const int yPos = y+height+borderWidth;
	const int tickRoom = y+h-yPos;
	myCanvas->tft->fillRect(x,y+height,w,h-height,bgColor);
	if(yPos < y || tickRoom <= 0) return;
	const int textY = min(yPos+10+FONT_Y,y+h-FONT_Y);
	const bool textFits = textY >= yPos+10;
	const int gap = 2;
	int rightX = 0, leftX = 0;
	const int rightWidth = xScaleLabel(0,rightX);
	const int leftWidth = span ? xScaleLabel(span,leftX) : 0;
	// Reserve the oldest endpoint only if it can coexist with the newest.
	const bool reserveLeft = leftWidth && (!rightWidth || leftX+leftWidth+gap <= rightX);
	int nextRight = x+w+gap;
	const int width = w-chartX()-2*borderWidth;
	const int minorStep = width >= 15 ? width/15 : 1;
	XAxisGlyphs glyphs(*myCanvas->tft);
	for(int offset = 0; offset <= span; ++offset){
		const int xp = getXVal(trendWindow.minValue+offset);
		if(xp < x || xp >= x+w) continue;
		bool labeled = false;
		// Preserve classic even-offset candidates and endpoints; thin using
		// their rendered rectangles, not an architecture-specific sample step.
		if(textFits && (offset == 0 || offset == span || offset%2 == 0)){
			int tx = 0;
			const int tw = xScaleLabel(offset,tx);
			if(tw && tx+tw+gap <= nextRight &&
			   (!reserveLeft || offset == span || tx >= leftX+leftWidth+gap)){
				for(int i = 0; buf[i]; ++i)
					glyphs.drawChar(tx+i*FONT_X,textY,
						static_cast<unsigned char>(buf[i]),borderColor,borderColor,1);
				nextRight = tx;
				labeled = true;
			}
		}
		if(labeled || offset == 0 || offset == span)
			myCanvas->tft->drawVerticalLine(xp,yPos,min(10,tickRoom),borderColor);
		else if(offset%minorStep == 0)
			myCanvas->tft->drawVerticalLine(xp,yPos,min(5,tickRoom),borderColor);
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

// Only committed geometry is erased. Pending history never reaches the display.
void TrendBase::resetRenderCache(){
    // init() releases projections, not the pixels already on the target.
    if(renderValid && renderedRadius && renderedPoints) {
        for(int s=0; s<renderedSeries; ++s) {
            const int mx=rendered[0];
            const int my=rendered[renderedPoints*(renderedSeries+1)+2*renderedSeries+s];
            const int right=max(renderStyle[0]+renderStyle[2],mx+renderedRadius+1);
            const int bottom=max(renderStyle[1]+renderStyle[3],my+renderedRadius+1);
            renderStyle[0]=min(renderStyle[0],mx-renderedRadius);
            renderStyle[1]=min(renderStyle[1],my-renderedRadius);
            renderStyle[2]=right-renderStyle[0]; renderStyle[3]=bottom-renderStyle[1];
        }
    }
    free(rendered); rendered = nullptr; renderedCapacity = 0;
    renderValid = false; renderInvalid = true;
}

void TrendBase::paintCommitted(bool erase){
    if(!rendered || !renderValid) return;
    const int n = renderedPoints, sCount = renderedSeries;
    const int* colors = rendered + n*(sCount+1);
    for(int s=0; s<sCount; ++s){
        const int* ys = rendered + n*(s+1);
        const int color = erase ? renderStyle[4] : colors[s];
        for(int j=n-1; j>0; --j){
            if(renderedSquare){
                myCanvas->tft->drawHorizontalLine(rendered[j],ys[j],rendered[j-1]-rendered[j]+1,color);
                myCanvas->tft->drawVerticalLine(rendered[j-1],min(ys[j],ys[j-1]),abs(ys[j]-ys[j-1])+1,color);
            }else myCanvas->tft->drawLine(rendered[j],ys[j],rendered[j-1],ys[j-1],color);
        }
    }
    // All lines and then all old pucks disappear before any new foreground.
    if(renderedRadius) for(int s=0; s<sCount; ++s){
        const int mx=rendered[0], my=rendered[n*(sCount+1)+2*sCount+s];
        myCanvas->tft->fillCircle(mx,my,renderedRadius,erase ? renderStyle[4] : colors[sCount+s]);
        if(erase && renderStyle[4]!=myCanvas->bgColor){
            // Widget background applies only inside the old widget. Restore
            // bounded outlying strips to the canvas before any new foreground.
            const int l=mx-renderedRadius, r=mx+renderedRadius+1;
            const int t=my-renderedRadius, b=my+renderedRadius+1;
            const int wl=renderStyle[0], wr=wl+renderStyle[2];
            const int wt=renderStyle[1], wb=wt+renderStyle[3];
            if(l<wl) myCanvas->tft->fillRect(l,t,min(r,wl)-l,b-t,myCanvas->bgColor);
            if(r>wr) myCanvas->tft->fillRect(max(l,wr),t,r-max(l,wr),b-t,myCanvas->bgColor);
            const int il=max(l,wl), ir=min(r,wr);
            if(ir>il && t<wt) myCanvas->tft->fillRect(il,t,ir-il,min(b,wt)-t,myCanvas->bgColor);
            if(ir>il && b>wb) myCanvas->tft->fillRect(il,max(t,wb),ir-il,b-max(t,wb),myCanvas->bgColor);
        }
    }
}

void TrendBase::eraseCommitted(){
    if(!canDraw() || !buf) return;
    if(renderedCanvas==myCanvas && renderedDisplay==myCanvas->tft) {
        if(renderValid) paintCommitted(true);
        else if(hasFrame && renderStyle[2]>0 && renderStyle[3]>0)
            myCanvas->tft->fillRect(renderStyle[0],renderStyle[1],renderStyle[2],renderStyle[3],myCanvas->bgColor);
    }
    renderInvalid = true;
}

void TrendBase::retainLegacyBounds(const uint8_t* data, int series, int radius){
    if(!canDraw() || !buf || w<=0 || h<=0) return;
    int left=x, top=y, right=x+w, bottom=y+h;
    if(data && maxValues && radius>0) for(int s=0;s<series;++s){
        const int mx=getXVal(trendWindow.minValue), my=getYVal(data[(size_t)s*maxValues]);
        left=min(left,mx-radius); top=min(top,my-radius);
        right=max(right,mx+radius+1); bottom=max(bottom,my+radius+1);
    }
    if(dirtyCanvas==myCanvas && dirtyDisplay==myCanvas->tft){
        left=min(left,dirtyBounds[0]); top=min(top,dirtyBounds[1]);
        right=max(right,dirtyBounds[2]); bottom=max(bottom,dirtyBounds[3]);
    }
    dirtyBounds[0]=left; dirtyBounds[1]=top; dirtyBounds[2]=right; dirtyBounds[3]=bottom;
    dirtyCanvas=myCanvas; dirtyDisplay=myCanvas->tft;
    invalidate();
}

void TrendBase::commitFrame(const uint8_t* data, uint8_t series, const int* colors,
                           int radius, int warn, int crit, int warnColor, int critColor, bool force){
    if(!canDraw() || !buf || !data || !maxValues) return;
    setWindow(trendWindow.minValue,trendWindow.maxValue);
    const int first = trendWindow.minValue, n = trendWindow.maxValue-first+1;
    // Exact comparison, not a hash: public style mutations cannot collide.
    const int style[] = {x,y,w,h,bgColor,borderColor,borderWidth,
        (int)scaleMin,(int)scaleMax,(int)setpoint,(int)hiLimit,(int)lowLimit,
        hiLimitColor,lowLimitColor,setpointColor,showXScale,showYScale,showBorder,
        showThresholds,showGridlines,rightInset,topInset,bottomInset,maxX,
        first,trendWindow.maxValue,maxValues,series,forceSquareWaveform,
        radius,warn,crit,warnColor,critColor,yScaleWidth,xScaleHeight};
    static_assert(sizeof(style)<=sizeof(renderStyle),"render style capacity");
    const bool sameTarget = renderedCanvas==myCanvas && renderedDisplay==myCanvas->tft;
    bool full = force || renderInvalid || !renderValid || !sameTarget ||
                memcmp(style,renderStyle,sizeof(style))!=0;
    if(w<=0 || h<=0){
        if(!full) return; // Empty states never compare or paint projected samples.
        if(dirtyCanvas==myCanvas && dirtyDisplay==myCanvas->tft)
            myCanvas->tft->fillRect(dirtyBounds[0],dirtyBounds[1],
                dirtyBounds[2]-dirtyBounds[0],dirtyBounds[3]-dirtyBounds[1],myCanvas->bgColor);
        dirtyCanvas=nullptr; dirtyDisplay=nullptr;
        if(sameTarget && hasFrame){
            paintCommitted(true);
            if(renderStyle[2]>0 && renderStyle[3]>0)
                myCanvas->tft->fillRect(renderStyle[0],renderStyle[1],renderStyle[2],renderStyle[3],myCanvas->bgColor);
        }
        memcpy(renderStyle,style,sizeof(style));
        renderedPoints=renderedSeries=renderedRadius=0;
        renderValid=true; renderInvalid=false; hasFrame=false;
        renderedCanvas=myCanvas; renderedDisplay=myCanvas->tft;
        return;
    }
    const size_t required = (size_t)n*(series+1)+3*series;
    bool changed = full;
    if(!changed){
        for(int i=0;i<n;++i) if(rendered[i]!=getXVal(first+i)) changed=true;
        for(int s=0;s<series;++s){
            for(int i=0;i<n;++i)
                if(rendered[n*(s+1)+i]!=getYVal(data[(size_t)s*maxValues+first+i])) changed=true;
            const int color = colors ? colors[s] : BLUE;
            const int v = data[(size_t)s*maxValues];
            const int mc = crit && v>=crit ? critColor : warn && v>=warn ? warnColor : color;
            if(rendered[n*(series+1)+s]!=color || rendered[n*(series+1)+series+s]!=mc ||
               (radius && rendered[n*(series+1)+2*series+s]!=getYVal(v))) changed=true;
        }
    }
    if(!changed) return;
    if(dirtyCanvas==myCanvas && dirtyDisplay==myCanvas->tft){
        myCanvas->tft->fillRect(dirtyBounds[0],dirtyBounds[1],
            dirtyBounds[2]-dirtyBounds[0],dirtyBounds[3]-dirtyBounds[1],myCanvas->bgColor);
    }
    dirtyCanvas=nullptr; dirtyDisplay=nullptr;
    if(sameTarget && hasFrame){
        paintCommitted(true);
        if(full && renderStyle[2]>0 && renderStyle[3]>0)
            myCanvas->tft->fillRect(renderStyle[0],renderStyle[1],renderStyle[2],renderStyle[3],myCanvas->bgColor);
    }
    if(required>renderedCapacity){
        // Failed growth keeps the allocation owned and selects a safe full redraw.
        int* replacement = (int*)realloc(rendered,required*sizeof(int));
        if(replacement){ rendered=replacement; renderedCapacity=required; }
        else full=true;
    }
    if(full && w>0 && h>0) myCanvas->tft->fillRect(x,y,w,h,bgColor);
    if(full){ drawYScale(); drawXScale(); }
    if(showGridlines) drawGridlines();
    drawThresholdLines(true);
    if(renderedCapacity>=required){
        for(int i=0;i<n;++i) rendered[i]=getXVal(first+i);
        for(int s=0;s<series;++s){
            for(int i=0;i<n;++i) rendered[n*(s+1)+i]=getYVal(data[(size_t)s*maxValues+first+i]);
            const int color = colors ? colors[s] : BLUE;
            const int v = data[(size_t)s*maxValues];
            rendered[n*(series+1)+s]=color;
            rendered[n*(series+1)+2*series+s]=getYVal(v);
            rendered[n*(series+1)+series+s]=crit && v>=crit ? critColor : warn && v>=warn ? warnColor : color;
        }
        memcpy(renderStyle,style,sizeof(style));
        renderedPoints=n; renderedSeries=series; renderedRadius=radius;
        renderedSquare=forceSquareWaveform; renderValid=true;
        renderedCanvas=myCanvas; renderedDisplay=myCanvas->tft; renderInvalid=false;
        paintCommitted(false);
    }else{
        // OOM fallback has no partial frame, no history loss and no stale cache.
        renderValid=false; renderInvalid=true;
        memcpy(renderStyle,style,sizeof(style));
        for(int s=0;s<series;++s) drawValuesSeries(data+(size_t)s*maxValues,colors?colors[s]:BLUE);
        if(radius) for(int s=0;s<series;++s){
            const int v=data[(size_t)s*maxValues], color=colors?colors[s]:BLUE;
            const int mx=getXVal(first), my=getYVal(v);
            myCanvas->tft->fillCircle(mx,my,radius,
                crit && v>=crit ? critColor : warn && v>=warn ? warnColor : color);
            // Without projected geometry, retain an eraseable bounding rectangle,
            // including public radii that extend beyond the widget itself.
            const int right=max(renderStyle[0]+renderStyle[2],mx+radius+1);
            const int bottom=max(renderStyle[1]+renderStyle[3],my+radius+1);
            renderStyle[0]=min(renderStyle[0],mx-radius);
            renderStyle[1]=min(renderStyle[1],my-radius);
            renderStyle[2]=right-renderStyle[0]; renderStyle[3]=bottom-renderStyle[1];
        }
    }
    hasFrame=true; renderedCanvas=myCanvas; renderedDisplay=myCanvas->tft;
    drawBorder(); // Restore the frame after dynamic content, including square waves.
}

int TrendBase::maxOf(const uint8_t* values){
	if(!values || !maxValues) return scaleMax;
	int val = values[0];
	for(int i = 1; i<maxValues; i++) if(values[i] > val) val = values[i];
	return val;
}
