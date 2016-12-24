#include "Screen.h" 

Screen::Screen(){
	this->init();
}

Screen::Screen(Canvas* c){
	canvas = c;
	this->init();
}

Screen::~Screen(){}

void Screen::init(){
	x=0;
	y=42;
	w = (canvas->_mode) ? 320 : 240;
	h = (canvas->_mode) ? 157 : 237;
	bgColor = canvas->bgColor;
}

void Screen::add(Widget* w, int x, int y){
	widgets.push(w);
	w->setCanvas(canvas);
	w->x = x;
	w->y = y;
}

void Screen::show(){
	if(this->canvas->currentScreen != this) return; // Do not draw if not canvas' current screen.
	
	Tft.fillRectangle(x,y,w,h,bgColor);
	byte cnt = this->widgets.count();
	for(int i=0;i<cnt;i++){
		if(this->widgets[i]->visible) this->widgets[i]->show();
	}	
}

void Screen::setCanvas(Canvas* c){
	canvas = c;
}

void Screen::touchWidgets(Point* p){
	byte cnt = widgets.count();
	char j;
	for(int i=1; i<=cnt; i++){
		j = cnt-i;
		if(widgets[j]->isButton && widgets[j]->visible){
			if(!widgets[j]->checkTouch(p)) break;  //Break if widget blocks event after handling it.
		}
	}
}
