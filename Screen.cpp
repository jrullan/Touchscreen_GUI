#include "Screen.h" 

Screen::Screen(){}

Screen::Screen(Canvas* c){
	canvas = c;
}

Screen::~Screen(){}

void Screen::init(){}

void Screen::add(Widget* w, int x, int y){
	widgets.push(w);
	w->setCanvas(canvas);
	w->x = x;
	w->y = y;
}

void Screen::show(){
	byte cnt = widgets.count();
	for(int i=0;i<cnt;i++){
		if(widgets[i]->visible) widgets[i]->show();
	}	
}

void Screen::hide(){
	byte cnt = widgets.count();
	for(int i=0;i<cnt;i++){
		widgets[i]->visible = false;
	}
	Tft.fillScreen();
}

void Screen::setCanvas(Canvas* c){
	canvas = c;
}

void Screen::touchWidgets(Point* p){
	byte cnt = widgets.count();
	for(int i=1; i<=cnt; i++){
		if(widgets[cnt-i]->isButton && widgets[cnt-i]->visible){
			if(!widgets[cnt-i]->checkTouch(p)) break;  //Break if widget blocks event after handling it.
		}
	}
}
