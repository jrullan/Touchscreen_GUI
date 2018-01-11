#include "Popup.h"

Popup::Popup(){
	if(text = (char *)malloc(8+1)) memset(text,0,8+1); //Had to add one more, to avoid some bug
}

//Popup::Popup(unsigned char type, char* message){
Popup::Popup(char* message){
	unsigned char text_length = getTextLength(message);
	if(text = (char *)malloc(text_length+1)) memset(text,0,text_length+1); //Had to add one more, to avoid some bug
	if(btn1Text = (char *)malloc(8+1)) memset(btn1Text,0,8+1); //Had to add one more, to avoid some bug
	if(btn2Text = (char *)malloc(8+1)) memset(btn2Text,0,8+1); //Had to add one more, to avoid some bug
	text = message;
	this->init();
}

Popup::~Popup(){
	if(text) free(text);
	if(btn1Text) free(btn1Text);
	if(btn2Text) free(btn2Text);
	
}

void Popup::init(){
	Button::init();
	type = 0x33;
	this->borderWidth = 2;
	this->fontSize = 2;
	this->visible = false; // This widget should not be visible by default
}

void Popup::drawFrame(int pX, int pY, int wW, int hH){
	for(unsigned char i=0; i<borderWidth; i++)
	{
		Tft.drawRectangle(pX+i,pY+i,wW-i*2,hH-i*2,borderColor);
	}
	//Serial.println("Drew frame");
}


void Popup::drawText(int pX, int pY, int wW, int hH, char* t){
	if(!t) return;
	int stringX = pX+wW/2-(getTextLength(t)*FONT_SPACE*fontSize)/2;
	int stringY = pY+hH/2-(FONT_Y*fontSize)/2;
	Tft.drawString(t,stringX,stringY,fontSize,fgColor);
	//Serial.println("Drew text");
}


void Popup::draw(){
	int btnPoX, btnPoY;
	
	if(popup_type==POPUP_OK){
		btn1Text = "Ok";
		btn2Text = 0;
	}
	if(popup_type==POPUP_YES_NO){
		btn1Text = "Yes";
		btn2Text = "No";
	}
	if(popup_type==POPUP_OK_CANCEL){
		btn1Text = "Ok";
		btn2Text = "Cancel";
	}
		
	// Draw enclosing frame and background
	drawFrame(x,y,w,h);
	Tft.fillRectangle(x+borderWidth,y+borderWidth,w-borderWidth*2,h-borderWidth*2,bgColor);
	
	// Draw message
	this->drawText(x,y,w,h-getBtnHeight(),text);
	//Serial.println(text);
	
	// Draw buttons (One or Two based on popup_type)
	btnPoY = y+h-getBtnHeight()-10;		
	if(popup_type > POPUP_OK){ // Two buttons
		btnPoX = x+w/2-(getBtnWidth()*2+10)/2;
		//Button 1
		drawFrame(btnPoX, btnPoY, getBtnWidth(), getBtnHeight());
		Tft.fillRectangle(btnPoX+borderWidth,btnPoY+borderWidth,getBtnWidth()-borderWidth*2,getBtnHeight()-borderWidth*2,GREEN);
		drawText(btnPoX,btnPoY,getBtnWidth(),getBtnHeight(),btn1Text);
			
		//Button 2
		drawFrame(btnPoX+getBtnWidth()+10,btnPoY,getBtnWidth(),getBtnHeight());
		Tft.fillRectangle(btnPoX+borderWidth+getBtnWidth()+10,btnPoY+borderWidth,getBtnWidth()-borderWidth*2,getBtnHeight()-borderWidth*2,0xf888); //Redish
		drawText(btnPoX+getBtnWidth()+10,btnPoY,getBtnWidth(),getBtnHeight(),btn2Text);
	
	}else{ // One button
		btnPoX = x+w/2-(getBtnWidth())/2;		
		//Button 1
		drawFrame(btnPoX,btnPoY,getBtnWidth(),getBtnHeight());
		Tft.fillRectangle(btnPoX+borderWidth,btnPoY+borderWidth,getBtnWidth()-borderWidth*2,getBtnHeight()-borderWidth*2,0x551f); // 0x001f Bluish
			// Btn1 Message
			drawText(btnPoX,btnPoY,getBtnWidth(),getBtnHeight(),btn1Text);
	}
	//Serial.println("Drew all");
}

int Popup::getBtnHeight(){
	return (fontSize*FONT_Y + 20);
}

int Popup::getBtnWidth(){
	return (DISPLAY_SIZE*fontSize*FONT_X)-30 / 2;
}

void Popup::setEventHandler(void (*functionPointer)(Popup*,unsigned char)){
	eventHandler = functionPointer;
} 


void Popup::setText(char* _text){
	text = _text;
}
/*
void Popup::setText(char* _text, char* _btn1, char* _btn2){
  text = _text;
  btn1Text = _btn1;
  btn2Text = _btn2;
}
*/


void Popup::processEvent(unsigned char btnNo){
	//process event
	selection = btnNo;
	eventHandler(this,btnNo); // Call event handler	& pass button number
	//Serial.println("Processing Event");	
}


// Overrides
bool Popup::checkTouch(Point* p){
	int boundX1, boundX2, boundY1, boundY2;
	int btnWidth, btnHeight;
	bool pressed = false;

	btnWidth = getBtnWidth();
	btnHeight = getBtnHeight();
	
	if(lastMillis + debounceTime < millis()){ 
		
		if((p->x > x+borderWidth) && (p->x < x+w-borderWidth) && (p->y > y+borderWidth) && (p->y < y+h-borderWidth)){
			//Serial.println("Popup received touch event");						
			// Check which button receives the event
			if(popup_type > POPUP_OK){
				//Button 1 event
				boundX1 = x+(w-(getBtnWidth()*2+10))/2;
				boundX2 = boundX1 + getBtnWidth();
				boundY1 = y+h-getBtnHeight()-10;
				boundY2 = boundY1 + h;
				if((p->x > boundX1)&&(p->x < boundX2) && (p->y > boundY1)&&(p->y < boundY2)){
					processEvent(1);
					//eventHandler(this,1);
					return false;
				}
				
				//Button 2 event
				boundX1 = boundX1 + getBtnWidth() + 10;//x+(w-(getBtnWidth()*2+10))/2;
				boundX2 = boundX1 + getBtnWidth();
				//boundY1 = y+h-getBtnHeight()-10;
				//boundY2 = boundY1 + h;
				if((p->x > boundX1)&&(p->x < boundX2) && (p->y > boundY1)&&(p->y < boundY2)){
					processEvent(2);
					//eventHandler(this,2);
					return false;
				}				
			
			}else{
				boundX1 = x+(w-getBtnWidth())/2;
				boundX2 = boundX1 + w;
				boundY1 = y+h-getBtnHeight()-10;
				boundY2 = boundY1 + h;
				if((p->x > boundX1)&&(p->x < boundX2) && (p->y > boundY1)&&(p->y < boundY2)){
					processEvent(1);
					//eventHandler(this,1);
					return false;
				}
			}
			//return false; // Return false to stop event from bubbling up.
		}// -- if touch within widget area
		touched = !touched;
		lastMillis = millis();		
	}// -- debounce
	return false; // <--- False means block further event checking.
}

//void Popup::show(void (*eH)(Button*),Button* btn){
void Popup::show(Button* btn){
	targetButton = btn;
	show();
}

void Popup::targetEventHandler(){
	targetButton->eventHandler(targetButton);
}

void Popup::show(){
	draw();
	update();
	this->visible = true;
}

void Popup::hide(){
	this->visible = false;	
	Tft.fillRectangle(x,y,w,h,myCanvas->bgColor);
	myCanvas->redraw();
}

void Popup::reset(){
	selection = NULL;
}

void Popup::update(){
	// ???
	return;
}
