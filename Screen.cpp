#include "Screen.h" 

Screen::Screen(){
	this->init();
}

Screen::Screen(Canvas* c, int _x, int _y, int _w, int _h){
	canvas = c;
	this->x = _x;
	this->y = _y;
	this->w = _w;
	this->h = _h;
	this->bgColor = canvas->bgColor;
	this->init();
}

Screen::~Screen(){}

void Screen::init(){
	//x=0;
	//y=42;
	//w = (canvas->_mode == TFT_LANDSCAPE) ? 320 : 240;
	//h = (canvas->_mode == TFT_PORTRAIT) ? 157 : 237;
	//bgColor = canvas->bgColor;
}

void Screen::add(Widget* widget, int wX, int wY){
	widgets.push(widget);
	widget->setCanvas(canvas);
	widget->x = wX + this->x;
	widget->y = wY + this->y;
}

void Screen::show(){
	if(this->canvas->currentScreen != this) return; // Do not draw if not canvas' current screen.
	
	canvas->tft->fillRect(x,y,w,h,bgColor);
	byte cnt = this->widgets.count();
	for(int i=0;i<cnt;i++){
		if(this->widgets[i]->visible) this->widgets[i]->show();
	}	
}

void Screen::setCanvas(Canvas* c){
	canvas = c;
}

void Screen::touchWidgets(Point* p){
	//Serial.println("Screen touchWidgets()");
	byte cnt = widgets.count();
	char j;
	for(int i=1; i<=cnt; i++){
		j = cnt-i;
		//Serial.print("Touching widget ");Serial.println(Widget::getType(widgets[j]));
		if(widgets[j]->isButton && widgets[j]->visible){
			//Serial.print("Checking touch for widget: ");Serial.println(Widget::getType(widgets[j]));
			if(!widgets[j]->checkTouch(p)) break;  //Break if widget blocks event after handling it.
		}
	}
}
