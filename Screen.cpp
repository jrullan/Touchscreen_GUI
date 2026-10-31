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
	this->bgColor = canvas ? canvas->bgColor : 0;
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
	if (!widget || widgets.contains(widget)) return;
	if (!widgets.push(widget)) return;
	widget->setCanvas(canvas);
	widget->x = wX + this->x;
	widget->y = wY + this->y;
	widget->visible = false;	// added to make the widgets not visible by default if in a screen to avoid accidentaly updating them
}

void Screen::hide(){
	//canvas->tft->fillRect(x,y,w,h,canvas->bgColor);
	int cnt = this->widgets.count();
	for(int i=0;i<cnt;i++){
		//if(this->widgets[i]->visible) this->widgets[i]->hide();
		if (widgets[i]) widgets[i]->visible = false;
	}	
}

void Screen::show(){
	if(!canvas || !canvas->tft || canvas->currentScreen != this) return; // Do not draw if not canvas' current screen.
	canvas->tft->fillRect(x,y,w,h,bgColor);
	int cnt = this->widgets.count();
	for(int i=0;i<cnt;i++){
		//if(this->widgets[i]->visible) this->widgets[i]->show();
		if (!widgets[i]) continue;
		this->widgets[i]->visible = true;
		this->widgets[i]->show();
	}	
}

void Screen::setCanvas(Canvas* c){
    if (canvas && canvas != c && canvas->currentScreen == this) canvas->setScreen(NULL);
    canvas = c;
    for (int i = 0; i < widgets.count(); ++i) {
        if (widgets[i]) widgets[i]->setCanvas(c);
    }
}

bool Screen::remove(Widget* widget) {
    if (!widget || !widgets.remove(widget)) return false;
    widget->setCanvas(NULL);
    return true;
}

Widget* Screen::pop() {
    if (widgets.isEmpty()) return NULL;
    Widget* widget = widgets.pop();
    if (widget) widget->setCanvas(NULL);
    return widget;
}

void Screen::detach() {
    hide();
    setCanvas(NULL);
}

void Screen::touchWidgets(Point* p){
	if (!p) return;
	//Serial.println("Screen touchWidgets()");
	int cnt = widgets.count();
	int j;
	for(int i=1; i<=cnt; i++){
		j = cnt-i;
		//Serial.print("Touching widget ");Serial.println(Widget::getType(widgets[j]));
		if(widgets[j] && widgets[j]->isButton && widgets[j]->visible){
			//Serial.print("Checking touch for widget: ");Serial.println(Widget::getType(widgets[j]));
			if(!widgets[j]->checkTouch(p)) break;  //Break if widget blocks event after handling it.
		}
	}
}
