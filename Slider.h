#ifndef SLIDER_H
#define SLIDER_H

#if !defined(DEBOUNCE)
#define DEBOUNCE 200
#endif

#include "Canvas.h"
#include "Indicator.h"

class Slider : public Indicator
{
	public: 
		Slider();
		~Slider();
		
		void setEventHandler(void (*functionPointer)(Slider *));
		void init();
		void drawBorder();
		void drawCV(int cv);
		void setDebounce(int debounce);
				
		// Overriden methods
		bool checkTouch(Point* p);
		void show();
        void hide();
		void update();
        // Explicit repaint requests retain the committed footprint for cleanup.
        void invalidate() { repaint=true; }
        void forceUpdate() { repaint=true; render(true); }

	void (*eventHandler)(Slider*) = NULL;
	bool block = false;
	unsigned long lastMillis = 0;
	
protected:	
    struct RenderState {
        Canvas* canvas = nullptr;
        Guitft* driver = nullptr;
        int style[10] = {};
        int top = 0;
        bool valid = false;
    } rendered;
    RenderState state(int64_t value) const;
    void render(bool force) { render(force,currentValue); }
    void render(bool force, int64_t value);
    void paintFill(int top);
    void paintBorder();
    bool repaint = false;
	unsigned int debounceTime = DEBOUNCE;	
};


inline Slider::Slider(){
	init();
};

inline Slider::~Slider(){}

inline void Slider::init(){
	type=0x24;
	borderWidth = 2;
	scaleMax = 100;
	scaleMin = 0;
	setHiLimit(scaleMax - 10,RED);
	setLowLimit(scaleMin + 10,BLUE);
	setSP(50,YELLOW);
	setSize(40,120);
	setColors(BLACK,BLUE,WHITE);
	this->isButton = true;
}

inline void Slider::drawBorder(){
    if(!canDraw() || w<=0 || h<=0 || borderWidth<=0) return;
    Guitft::ScopedWrite write(*myCanvas->tft);
    paintBorder();
    RenderState next=state(currentValue);
    if(rendered.valid && rendered.canvas==next.canvas && rendered.driver==next.driver &&
       rendered.style[2]>0 && rendered.style[3]>0) {
        const int right=max(x+w,rendered.style[0]+rendered.style[2]);
        const int bottom=max(y+h,rendered.style[1]+rendered.style[3]);
        next.style[0]=min(x,rendered.style[0]); next.style[1]=min(y,rendered.style[1]);
        next.style[2]=right-next.style[0]; next.style[3]=bottom-next.style[1];
    }
    rendered=next; repaint=true;
}
inline void Slider::paintBorder(){
	if (!canDraw() || w<=0 || h<=0) return;
	for(int i=0; i<borderWidth && w-2*i>0 && h-2*i>0; ++i){
		myCanvas->tft->drawRect(x+i,y+i,w-i*2,h-i*2,borderColor);
	}	
}

inline void Slider::drawCV(int cv){ render(false,cv); }
inline void Slider::paintFill(int top){
	if (!canDraw() || w<=0 || h<=0) return;
    const int bw=borderWidth>0?borderWidth:0;
    if(w-2*bw<=0 || h-2*bw<=0) return;
	int cvY = y+top;
	
	//		Draw the background
	int startFill = y+bw;
	int fillHeight = cvY - startFill;
    if(fillHeight>0) myCanvas->tft->fillRect(x+bw,startFill,w-2*bw,fillHeight,bgColor);

	//		Draw the knob/marker
	startFill = cvY;
	fillHeight = y+h-bw-startFill;
    if(fillHeight>0) myCanvas->tft->fillRect(x+bw,startFill,w-2*bw,fillHeight,fgColor);
}

inline void Slider::setDebounce(int debounce){
	this->debounceTime = debounce;
}

inline void Slider::setEventHandler(void (*functionPointer)(Slider *)){
	eventHandler = functionPointer;
}

inline bool Slider::checkTouch(Point* p){
	// Early exit if no event handler was defined
	if (!p || !canDraw() || !eventHandler) return false;
	
	if(lastMillis + debounceTime < millis()){ 
		// 1. Check if touch event falls within the bounds of this widget
		if((p->x > x) && (p->x < x+w) && (p->y > y) && (p->y < y+h)){
			// 1. Map the point->y position to the current Value (CV)			
			int range = h - borderWidth*2;
			int minY = y + borderWidth;
			int maxY = minY + range;
			int sValue = map(p->y,maxY,minY,scaleMin,scaleMax);
			setCV(sValue,true);
			
            // setCV above already committed the clamped value. Do not paint a
            // second raw value or advance the render baseline a second time.
			
			// 3. Call this widget's event handler routine
			eventHandler(this);
			
			// 4. Update debouncing timestamp
			lastMillis = millis();
			return true;
		}
	}
	// if block is true return false, so canvas will not continue to 
	// process the event.
	return !block;
}

inline Slider::RenderState Slider::state(int64_t value) const {
    RenderState s;
    s.canvas=myCanvas; s.driver=myCanvas ? myCanvas->tft : nullptr;
    const int fields[]={x,y,w,h,borderWidth,bgColor,fgColor,borderColor,
        (int)scaleMin,(int)scaleMax};
    memcpy(s.style,fields,sizeof(fields));
    const int bw=borderWidth>0?borderWidth:0;
    s.top=(w>2*bw && h>2*bw) ? mapScale(value,h-bw,bw) : 0;
    s.valid=true; return s;
}
inline void Slider::update(){ render(false); }
inline void Slider::render(bool force, int64_t value){
    if(!canDraw()) return;
    const RenderState next=state(value);
    if(!force && !repaint && rendered.valid && rendered.canvas==next.canvas && rendered.driver==next.driver &&
       memcmp(rendered.style,next.style,sizeof(next.style))==0 && rendered.top==next.top) return;
    const bool stable=rendered.valid && rendered.canvas==next.canvas &&
        rendered.driver==next.driver && memcmp(rendered.style,next.style,sizeof(next.style))==0;
    const bool sameTarget=rendered.valid && rendered.canvas==next.canvas && rendered.driver==next.driver;
    if((w<=0 || h<=0) && (!sameTarget || rendered.style[2]<=0 || rendered.style[3]<=0)) {
        rendered=next; repaint=false; return;
    }
    Guitft::ScopedWrite write(*myCanvas->tft);
    if(stable && !force && !repaint) {
        const int bw=borderWidth>0?borderWidth:0;
        const int width=w-2*bw;
        if(w>0 && h>2*bw && width>0 && next.top!=rendered.top)
            myCanvas->tft->fillRect(x+bw,y+min(next.top,rendered.top),
                width,abs(next.top-rendered.top),next.top<rendered.top?fgColor:bgColor);
    } else {
        if(rendered.valid && rendered.canvas==next.canvas && rendered.driver==next.driver &&
           rendered.style[2]>0 && rendered.style[3]>0)
            myCanvas->tft->fillRect(rendered.style[0],rendered.style[1],
                rendered.style[2],rendered.style[3],myCanvas->bgColor);
        if(w>0 && h>0) {
            myCanvas->tft->fillRect(x,y,w,h,bgColor);
            paintBorder(); paintFill(next.top);
        }
    }
    rendered=next; repaint=false;
}

inline void Slider::hide(){
    if(canDraw() && rendered.valid && rendered.canvas==myCanvas && rendered.driver==myCanvas->tft &&
       rendered.style[2]>0 && rendered.style[3]>0) {
        Guitft::ScopedWrite write(*myCanvas->tft);
        myCanvas->tft->fillRect(rendered.style[0],rendered.style[1],
            rendered.style[2],rendered.style[3],myCanvas->bgColor);
    }
    visible=false; repaint=true;
}
inline void Slider::show(){
	visible = true;
    render(true);
}

#endif
