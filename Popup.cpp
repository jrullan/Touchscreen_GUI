#include "Popup.h"

Popup::Popup(){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
}

Popup::Popup(unsigned char type, char* message){
	unsigned char text_length = getTextLength(message);
	if(text = (char *)malloc(text_length+1)) memset(text,0,text_length+1); //Had to add one more, to avoid some bug
	
	if(this->getTextSize() > text_length){
		this->setText("Err");
	}else{
		this->setText(message);
	}
	
	this->init();
}

Popup::~Popup(){
	if(text) free(text);
}

void Popup::init(){
	this->borderWidth = 2;
	this->fontSize = 2;
	this->visible = false; // This widget should not be visible by default
}

void Popup::drawFrame(int pX, int pY, int wW, int hH){
	for(unsigned char i=0; i<borderWidth; i++)
	{
		Tft.drawRectangle(pX+i,pY+i,wW-i*2,hH-i*2,borderColor);
	}	
}

void Popup::draw(){
	if(!this->visible)return;
	int btnPoX, btnPoY;
	
	// Draw enclosing frame and background
	drawFrame(x,y,w,h);
	Tft.fillRectangle(x+borderWidth,y+borderWidth,w-borderWidth*2,h-borderWidth*2,bgColor);
	
	// Draw message
	this->drawText(x,y,w,h-getBtnHeight(),text);
	//Serial.println(text);
	
	// Draw buttons (One or Two based on type)
	btnPoY = y+h-getBtnHeight()-10;
	if(type == POPUP_TWO_BUTTONS){ // Two buttons
		btnPoX = x+w/2-(getBtnWidth()*2+10)/2;
		//Button 1
		drawFrame(btnPoX, btnPoY, getBtnWidth(), getBtnHeight());
		Tft.fillRectangle(btnPoX+borderWidth,btnPoY+borderWidth,getBtnWidth()-borderWidth*2,getBtnHeight()-borderWidth*2,GRAY1);
		drawText(btnPoX,btnPoY,getBtnWidth(),getBtnHeight(),"Ok");
			
		//Button 2
		drawFrame(btnPoX+getBtnWidth()+10,btnPoY,getBtnWidth(),getBtnHeight());
		Tft.fillRectangle(btnPoX+borderWidth+getBtnWidth()+10,btnPoY+borderWidth,getBtnWidth()-borderWidth*2,getBtnHeight()-borderWidth*2,GRAY1);
		drawText(btnPoX+getBtnWidth()+10,btnPoY,getBtnWidth(),getBtnHeight(),"Cancel");
	
	}else{ // One button
		btnPoX = x+w/2-(getBtnWidth())/2;		
		//Button 1
		drawFrame(btnPoX,btnPoY,getBtnWidth(),getBtnHeight());
		Tft.fillRectangle(btnPoX+borderWidth,btnPoY+borderWidth,getBtnWidth()-borderWidth*2,getBtnHeight()-borderWidth*2,GRAY1);
			// Btn1 Message
			drawText(btnPoX,btnPoY,getBtnWidth(),getBtnHeight(),"Ok");
	}
	

}

void Popup::drawText(int pX, int pY, int wW, int hH, char* t){
	//Calculate centered position of the text
	//int textSize = (getTextLength(t)*FONT_SPACE*fontSize);
	//int offset = (wW - textSize)/2;
	/*Serial.print("pX,wW: ");
	Serial.print(pX);Serial.print(",");Serial.print(wW);
	Serial.print(" Text: ");
	Serial.print(t);
	Serial.print(" size,textSize,offset: ");
	Serial.print(getTextLength(t));Serial.print(",");Serial.print(textSize);
	Serial.print(",");Serial.println(offset);
	*/
	int stringX = pX+wW/2-(getTextLength(t)*FONT_SPACE*fontSize)/2;
	int stringY = pY+hH/2-(FONT_Y*fontSize)/2;
	Tft.drawString(t,stringX,stringY,fontSize,fgColor);
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

// Overrides
bool Popup::checkTouch(Point* p){
	int boundX1, boundX2, boundY1, boundY2;
	int btnWidth, btnHeight;
	bool pressed = false;

	btnWidth = getBtnWidth();
	btnHeight = getBtnHeight();
	
	if(lastMillis + debounceTime < millis()){ 
		//Serial.println("Popup received touch event");
		if((p->x > x+borderWidth) && (p->x < x+w-borderWidth) && (p->y > y+borderWidth) && (p->y < y+h-borderWidth)){
			Serial.println("Popup Touched!");
			this->visible = false; // Make widget not visible
			Tft.fillScreen(); // Erase screen (Black out)
			eventHandler(this,1); // Call event handler

			return false; // Return false to stop event from bubbling up.
		}// -- if touch within widget area
		touched = !touched;
		lastMillis = millis();		
	}// -- debounce
	return true; // <--- False means block further event checking.
}

bool Popup::isButton(){
	return true;
}

void Popup::show(){
	draw();
	update();
}

void Popup::update(){
	// ???
	return;
}
