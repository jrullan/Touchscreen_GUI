#include "Screen.h" 

Screen::Screen(){}

Screen::Screen(Canvas* c){
	canvas = c;
}

Screen::~Screen(){}

void Screen::init(){
	
}

void Screen::add(Widget* w, int x, int y){
	widgets.push(w);
	w->setCanvas(canvas);
	w->x = x;
	w->y = y;
	//Serial.print("Pushed widget type: ");Serial.println(Widget::getType(w->type));
}

void Screen::show(){
	byte cnt = this->widgets.count();
	for(int i=0;i<cnt;i++){
		if(this->widgets[i]->visible) this->widgets[i]->show();
	}	
}


/* 
 */
void Screen::hide(){
	Tft.fillScreen();
}

void Screen::setCanvas(Canvas* c){
	canvas = c;
}

void Screen::touchWidgets(Point* p){
	byte cnt = widgets.count();
	char j;
	for(int i=1; i<=cnt; i++){
		j = cnt-i;
		
		Serial.print("Sending touch event to ");
		Serial.println(Widget::getType(widgets[j]));
		
		if(widgets[j]->isButton && widgets[j]->visible){
			if(!widgets[j]->checkTouch(p)) break;  //Break if widget blocks event after handling it.
		}
	}
	//Serial.println("Screen processed the touch event");
}
