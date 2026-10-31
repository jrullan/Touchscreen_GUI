#include "Gauge.h"
#include <stdio.h>
//Constructor
Gauge::Gauge(){
	init();
}

Gauge::Gauge(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit)
{	
	this->setSize(width,height);
	this->setLimits(minLimit,setpoint,maxLimit);
	this->hiLimit = scaleMax;
	this->lowLimit = scaleMin;
	this->currentValue = this->setpoint;
	this->setColors(BLACK,BLUE,WHITE);
	init();
}

Gauge::~Gauge(){}

//Methods
void Gauge::init(){
	Indicator::init();
	type = 0x22;
	borderWidth=2;
	if(this->hiLimit == 0) this->hiLimit = scaleMax;
	if(this->lowLimit == 0)this->lowLimit = scaleMin;
	if(this->currentValue == 0)this->currentValue = setpoint;
}

void Gauge::clear(){
	Indicator::clear();
}

void Gauge::drawLimits(){
}

void Gauge::drawScale(){
	if(!canDraw() || !buf) return;
//=======================================
	//Draw Y SCALE
	int textWidth = 3 * FONT_X;
	int lineX = x + textWidth - 1; //to separate from border
	
	// Draw background
	//myCanvas->tft->fillRect(x,y,yScaleWidth-borderWidth+1,h+borderWidth,bgColor);
	
	// Draw Max value and line
	setNum(scaleMax); // Sets buf with scaleMax value
	myCanvas->tft->drawString(buf,x,getYVal(scaleMax),1,borderColor);
	myCanvas->tft->drawHorizontalLine(lineX,getYVal(scaleMax),10,borderColor);
	
	// Draw Setpoint value and line
	setNum(setpoint); // Sets buf with setpoint value
	myCanvas->tft->drawString(buf,x,getYVal(setpoint)-(FONT_Y/2),1,setpointColor);
	myCanvas->tft->drawHorizontalLine(lineX,getYVal(setpoint),10,setpointColor);
	
	// Draw Min value and line
	setNum(scaleMin); // Sets buf with scaleMin value
	myCanvas->tft->drawString(buf,x,getYVal(scaleMin)-(FONT_Y),1,borderColor);
	myCanvas->tft->drawHorizontalLine(lineX,getYVal(scaleMin),10,borderColor);

	// Draw HI and LOW limits values and lines
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

int Gauge::getYVal(int value){
	return getYValValue(value);
}

int Gauge::getYVal(unsigned int value){
	return getYValValue(value);
}

int Gauge::getYValValue(int64_t value){
	const int last = h > 0 ? h-1 : 0;
	const int top = constrain(borderWidth+1, 0, last);
	const int bottom = constrain(h-borderWidth, top, last);
	return y + mapScale(value, bottom, top);
}

//Overriden virtual methods
void Gauge::hide(){
    if(canDraw() && rendered.valid && rendered.canvas==myCanvas && rendered.driver==myCanvas->tft &&
       rendered.right>rendered.left && rendered.down>rendered.up) {
        Guitft::ScopedWrite write(*myCanvas->tft);
        myCanvas->tft->fillRect(rendered.left,rendered.up,
            rendered.right-rendered.left,rendered.down-rendered.up,myCanvas->bgColor);
    }
    visible=false; repaint=true;
}
void Gauge::show(){
    visible = true;
    render(true);
}

void Gauge::drawBorder(){
    if(!canDraw() || !buf || w<=0 || h<=0 || borderWidth<=0) return;
    Guitft::ScopedWrite write(*myCanvas->tft);
    paintBorder();
    RenderState next=state();
    if(rendered.valid && rendered.canvas==next.canvas && rendered.driver==next.driver) {
        next.left=min(next.left,rendered.left); next.up=min(next.up,rendered.up);
        next.right=max(next.right,rendered.right); next.down=max(next.down,rendered.down);
    }
    rendered=next; repaint=true;
}
void Gauge::paintBorder(){
	if(!canDraw() || !buf) return;
	int xPos = x + yScaleWidth;
	int width = w;
	int yPos = y;
	int height = h;
	
	//Draw the Vertical Bar
	//--border
	for(int i=borderWidth; i>0 && width>0 && height>0; --i){
		myCanvas->tft->drawRect(xPos++,yPos++,width--,height--,borderColor);
		width--;
		height--;
	}
}

void Gauge::drawFill(){ render(false); }
void Gauge::paintFill(){
	if(!canDraw() || !buf || w <= 0 || h <= 0) return;
	const int bw = borderWidth > 0 ? borderWidth : 0;
	const int width = w-2*bw, bottom = h-bw;
	if(width <= 0 || bottom <= bw) return;
	const int val = mapScale(currentValue, bottom, bw);
	if(val > bw) myCanvas->tft->fillRect(x+bw+yScaleWidth,y+bw,width,val-bw,bgColor);
	int color=fgColor;
	if(currentValue >= hiLimit) color = hiLimitColor;
	if(currentValue <= lowLimit) color = lowLimitColor;
	if(bottom > val) myCanvas->tft->fillRect(x+bw+yScaleWidth,y+val,width,bottom-val,color);
}

Gauge::RenderState Gauge::state() {
    RenderState s;
    s.canvas=myCanvas; s.driver=myCanvas ? myCanvas->tft : nullptr;
    const int fields[] = {x,y,w,h,borderWidth,bgColor,fgColor,borderColor,
        (int)scaleMin,(int)scaleMax,(int)setpoint,(int)hiLimit,(int)lowLimit,
        hiLimitColor,lowLimitColor,setpointColor,yScaleWidth};
    memcpy(s.style,fields,sizeof(fields));
    const int bw=borderWidth>0?borderWidth:0;
    s.top=mapScale(currentValue,h-bw,bw);
    s.color=(uint16_t)(currentValue<=lowLimit ? lowLimitColor :
        (currentValue>=hiLimit ? hiLimitColor : fgColor));
    if(w<=2*bw || h<=2*bw) { s.top=0; s.color=0; }
    if(!repaint && rendered.valid && rendered.canvas==s.canvas && rendered.driver==s.driver &&
       memcmp(rendered.style,s.style,sizeof(s.style))==0) {
        // Value-only commits cannot change the scale-label/frame footprint.
        s.left=rendered.left; s.up=rendered.up; s.right=rendered.right; s.down=rendered.down;
        s.valid=true; return s;
    }
    if(w>0 && h>0 && s.driver) {
        s.left=x; s.up=y; s.right=x+w+yScaleWidth; s.down=y+h;
        const unsigned int labels[]={scaleMax,setpoint,scaleMin,hiLimit,lowLimit};
        for(int i=0;i<5;++i) {
            if((i==3 && hiLimit>=scaleMax) || (i==4 && lowLimit<=scaleMin)) continue;
            int tx=(int16_t)(uint16_t)x;
            int ty=(int16_t)(uint16_t)(getYVal(labels[i])-(i==0?0:(i==2?FONT_Y:FONT_Y/2)));
            char text[BUF_SIZE+1]; snprintf(text,sizeof(text),"%u",labels[i]);
            // Guitft uses the classic 6x8 size-one font; include screen wrapping.
            for(const char* p=text;*p;++p) {
                if(tx+FONT_X>s.driver->width()) { tx=0; ty+=FONT_Y; }
                s.left=min(s.left,tx); s.up=min(s.up,ty);
                s.right=max(s.right,tx+FONT_X); s.down=max(s.down,ty+FONT_Y);
                tx+=FONT_X;
            }
            s.right=max(s.right,x+27);
        }
        s.left=constrain(s.left,0,(int)s.driver->width());
        s.right=constrain(s.right,s.left,(int)s.driver->width());
        s.up=constrain(s.up,0,(int)s.driver->height());
        s.down=constrain(s.down,s.up,(int)s.driver->height());
    }
    s.valid=true; return s;
}
void Gauge::update(){ render(false); }
void Gauge::render(bool force){
    if(!canDraw() || !buf) return;
    const RenderState next=state();
    if(!force && !repaint && rendered.valid && rendered.canvas==next.canvas && rendered.driver==next.driver &&
       memcmp(rendered.style,next.style,sizeof(next.style))==0 &&
       rendered.top==next.top && rendered.color==next.color) return;
    const bool stable=rendered.valid && rendered.canvas==next.canvas &&
        rendered.driver==next.driver && memcmp(rendered.style,next.style,sizeof(next.style))==0;
    const bool sameTarget=rendered.valid && rendered.canvas==next.canvas && rendered.driver==next.driver;
    if(next.right<=next.left && (!sameTarget || rendered.right<=rendered.left)) {
        rendered=next; repaint=false; return;
    }
    Guitft::ScopedWrite write(*myCanvas->tft);
    if(stable && !force && !repaint && rendered.color==next.color) {
        const int bw=borderWidth>0?borderWidth:0;
        const int width=w-2*bw;
        if(w>0 && h>2*bw && width>0 && next.top!=rendered.top)
            myCanvas->tft->fillRect(x+bw+yScaleWidth,y+min(next.top,rendered.top),
                width,abs(next.top-rendered.top),next.top<rendered.top?next.color:bgColor);
    } else if(stable && !force && !repaint) {
        paintFill(); // crossing a threshold repaints the entire affected fill
    } else {
        // Never dereference or write a formerly attached target.
        if(rendered.valid && rendered.canvas==next.canvas && rendered.driver==next.driver &&
           rendered.right>rendered.left && rendered.down>rendered.up)
            myCanvas->tft->fillRect(rendered.left,rendered.up,
                rendered.right-rendered.left,rendered.down-rendered.up,myCanvas->bgColor);
        if(next.right>next.left && next.down>next.up) {
            myCanvas->tft->fillRect(next.left,next.up,next.right-next.left,next.down-next.up,bgColor);
            drawScale(); paintBorder(); paintFill();
        }
    }
    rendered=next; repaint=false;
}
