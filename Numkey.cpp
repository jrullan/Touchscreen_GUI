#include "Numkey.h"

Numkey::Numkey(){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
	this->init();
}

Numkey::Numkey(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
	this->init();

	this->setSize(width,height);
	this->setColors(backgroundColor,textColor,borderColor);
}

Numkey::~Numkey(){
	if(text) free(text);
}

void Numkey::init(){
	Button::init();
	type = 0x32;
	borderWidth = 2;
	charPos = 0;
	setSize(120,180);
	debounceTime = 100;
	autoremove = true;
}

void Numkey::drawFrame(){
  int xPos,yPos,width,height;
  int btnWidth,btnHeight;
  
  xPos = x;	
  yPos = y;
  width = w;
  height = h;
  btnWidth = w / 3;
  btnHeight = h / 5;
  
  //--display background
  myCanvas->tft->fillRect(xPos,yPos,btnWidth*2,btnHeight,BLACK);
  //--backspace background
  myCanvas->tft->fillRect(xPos+btnWidth*2,yPos,btnWidth,btnHeight,bgColor);
  //--nums background rectangle
  myCanvas->tft->fillRect(xPos,yPos+btnHeight,width,height-btnHeight,bgColor);
  
	//-- outer border
  for(byte i=borderWidth; i!=0;i--){
    myCanvas->tft->drawRect(xPos,yPos,width,height,borderColor);
    xPos++;
    yPos++;
    width--;
    height--;
  }
  
  //-- horizontal lines
  for(byte j=1;j<5;j++)
  {
  	for(byte i=0; i<borderWidth;i++){
    	myCanvas->tft->drawHorizontalLine(x,y+btnHeight*j-1+i,width+borderWidth,borderColor);
  	}
  }
  
  //-- vertical lines
  for(byte j=1;j<3;j++)
  {
  	for(byte i=0; i<borderWidth;i++){
    	myCanvas->tft->drawVerticalLine(x+btnWidth*j-1+i,y+btnHeight,height-btnHeight+borderWidth,borderColor);
  	}
  }

	//-- draw numbers  
  xPos = btnWidth/2 - FONT_X;
  yPos = btnHeight/2 - FONT_Y;
  byte xF=0;
  byte yF=1;
  myCanvas->tft->drawString("<-",x+xPos+2*btnWidth-6,y+yPos,fontSize,fgColor); 
  for(byte number=1; number<10; number++)
  {
  	myCanvas->tft->drawNumber(number,x+xPos+(xF++)*btnWidth,y+yPos+(yF*btnHeight),fontSize,fgColor);
  	if(number == 3 || number == 6 || number == 9){
  		yF++;
  		xF = 0;
  	}
  }
  myCanvas->tft->drawString(".",x+xPos+(xF++)*btnWidth,y+yPos+(yF*btnHeight),fontSize,fgColor);  
  myCanvas->tft->drawNumber(0,x+xPos+(xF++)*btnWidth,y+yPos+(yF*btnHeight),fontSize,fgColor);
  myCanvas->tft->drawString("=",x+xPos+(xF++)*btnWidth,y+yPos+(yF*btnHeight),fontSize,fgColor);
}

void Numkey::append(char* c){
	byte cSize = getTextLength(c);
	byte txtSize = getTextLength(text);//getTextSize();
	byte space = DISPLAY_SIZE - txtSize;
	
	//Check that there space available to append
	if(txtSize < DISPLAY_SIZE){
		//Serial.print("Space available: ");Serial.println(space);
		for(int i=0; i<space; i++)
		{
			text[i+txtSize] = c[i];
		}
	}
	update();
}

void Numkey::appendNum(unsigned char c){
	//Serial.print("Append num ");Serial.println((byte)c);

	if(c > 9){ 
		//Serial.print("Not a number");Serial.write(c);
		return; //Exit if c is not a number 0 - 9
	}
	byte txtSize = getTextLength(text);//getTextSize();
	byte space = DISPLAY_SIZE - txtSize;
	//Check that there space available to append
	if(txtSize < DISPLAY_SIZE){
		text[txtSize] = c + 48;
		text[txtSize + 1] = 0;
		update();
	}
}

void Numkey::fitToText(){
  if(*text){
    char* chars = text;
    char size = 0;
    while(*chars){
      *chars++;
      size++;
    }
    w = size * FONT_SPACE * borderWidth + FONT_SPACE;
    h = FONT_Y * borderWidth + FONT_Y;
    //drawString(text,x+5,y+5,2,textColor);
  }
}

long Numkey::getNum(){
	char size = getTextSize();
	long result = 0;
	for(int i = 0; i<size; i++){
		if(text[i] == '.') break;  // Only process integer side
		result = result * 10 + text[i]-'0';
	}
	return result;
}

char* Numkey::getText(){
  return text;
}

byte Numkey::getTextLength(char* c){
	char size = 0;
	if(*c){
    char* chars = c;
    while(*chars){
      *chars++;
      size++;
    }
  }
  return size;
}

byte Numkey::getTextSize(){
  return getTextLength(text);
}

void Numkey::setNum(int num){
	clear();
	char numChar[DISPLAY_SIZE];
	char chars = 0;
	
	// Extract characters representing the powers of ten
	while(num >= 10)
	{
		numChar[chars++] = num%10;
		num /= 10;
		//Serial.print("num ");Serial.println(num);
	}
	
	numChar[chars++] = num;
	
	for(int j = 0; j < chars; j++)//DISPLAY_SIZE; j++)
	{
		text[chars-1-j] = '0'+numChar[j];
		//Serial.print("text[i] ");Serial.println(text[j]);
	}
	text[chars]=0;
	//update();
}

void Numkey::setEventHandler(void (*functionPointer)(Numkey *)){
	eventHandler = functionPointer;
}

/*
void Numkey::setText(char* _text){
	
	for(int i=0; i<DISPLAY_SIZE-1;i++){
		text[i] = _text[i];
	}
	update();
  //Serial.print("Set text to ");Serial.println(text);
}
*/

void Numkey::deleteChar(){
	int btnWidth = w / 3;
  int btnHeight = h / 5;
	byte textSize = getTextSize();
	//Serial.print("Size is ");Serial.println(textSize);
	if(!textSize) return;

	// last character, forcefully erase it now because update wont
	if(textSize==1){
		myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,(btnWidth*2)-borderWidth,btnHeight-borderWidth*2,BLACK);  
	}

	// clear character in last position
	for(int i = textSize-1; i >= 0; i--)
	{
		if(text[i] != 0){
			text[i] = 0;
			break;
		}
	}
	
	update();
}

void Numkey::clear(){
	byte textSize = getTextSize();
	if(textSize){
		for(int i = textSize-1; i >= 0; i--)
		{
			text[i] = 0;
		}
	}
	
  int btnWidth = w / 3;
  int btnHeight = h / 5;
  // clear area
  if(!autoremove){
		myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,(btnWidth*2)-borderWidth,btnHeight-borderWidth*2,BLACK);
  }else{
		update();
  }
}

//Overriden virtual methods

bool Numkey::checkTouch(Point* p){
	int boundX1, boundX2, boundY1, boundY2;
	int btnWidth = w/3;
	int btnHeight = h/5;
	bool pressed = false;
	if(lastMillis + debounceTime < millis()){ 
		if((p->x > x+borderWidth) && (p->x < x+w-borderWidth) && (p->y > y+borderWidth) && (p->y < y+h-borderWidth)){
			//Serial.println("Numkey event within bounds");
			//backspace coordinates
			boundX1 = x+btnWidth*2;
			boundX2 = x+w;
			boundY1 = y+borderWidth;
			boundY2 = y+btnHeight;
			if((p->x > boundX1) && (p->x < boundX2) && (p->y > boundY1) && (p->y < boundY2)){
				//Serial.println("Backspace pressed");
				deleteChar();
				pressed = true;
			}
			//num coordinates
			for(int r = 1;(r <= 4)&&(!pressed); r++)
			{
				boundY1 = y+(btnHeight)*r+borderWidth;
				boundY2 = y+(btnHeight)*(r+1)-borderWidth;
				for(int c = 1;(c <= 3)&&(!pressed);c++)
				{
					boundX1 = x + (btnWidth)*(c-1) + borderWidth;
					boundX2 = x + (btnWidth)*c - borderWidth;
					int num = 3*(r - 1) + c;

					if((p->x > boundX1) && (p->x < boundX2) && (p->y > boundY1) && (p->y < boundY2)){
						//Serial.print(num); Serial.println();
						//Serial.println("A number has been pressed");
						
						if(num==11){
							//Serial.println("0 pressed");
							num = 0;
						}
						
						if(num==10){
							//Serial.println(". pressed");
							append(".");
						}else if(num==12){
							//Serial.println("= pressed");
							entry = getNum();
							if(autoremove){
								if(myCanvas->widgets.peek() == this){ // check if top widget is this numkey
									myCanvas->pop();				  // Remove this numkey from the canvas...
									//this->hide();
								}
							}
							eventHandler(this);		// <<<------ Event handler called when = signed is pressed
						}else{
							appendNum(char(num));//setNum(num);
						}
						
						
						pressed = true;
					}
				}// -- columns for loop
			}// --rows for loop
		}// -- if touch within widget area
		touched = !touched;
		lastMillis = millis();		
	}// -- debounce
	return false;
}

void Numkey::show(){
	drawFrame();
	update();
	if(!visible) visible = true;
}

void Numkey::show(Button* btn){
	targetButton = btn;
	show();
}

void Numkey::targetEventHandler(){
	targetButton->eventHandler(targetButton);
}

void Numkey::reset(){
	entry = NULL;
}

/* New method to make it easier to use,
 * automatically redraw the canvas when hidden.
 */
void Numkey::hide(){
	//Widget::hide();
	myCanvas->tft->fillRect(x,y,w,h,this->myCanvas->bgColor);
	visible = false;
	this->myCanvas->redraw();
}

void Numkey::update(){
  int btnWidth = w / 3;
  int btnHeight = h / 5;
  
  if(*text){
    char* chars = text;
		
		//Count characters to center on the button - Nice trick from the myCanvas->tft2 library
    int size = 0;
    while(*chars){
      *chars++;
      size++;
    }
    //Calculate centered position of the text
    int stringX = getCenterTextX(x,btnWidth*2,size);
    int stringY = getCenterTextY(y,btnHeight);

		// clear area
		myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,(btnWidth*2)-borderWidth,btnHeight-borderWidth*2,BLACK);  

    // draw remaining characters
		myCanvas->tft->drawString(text,stringX,stringY,borderWidth,GREEN);
  }
}
