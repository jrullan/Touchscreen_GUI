#include "Buttongrid.h"

/*
Buttongrid::Buttongrid(unsigned char size){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); 
	columns = size;
	rows = size;
	gridSize = size;

	//Allocate memory for labels and states
	if(rows >= 0 && columns >= 0){
		char qty = rows * columns;
		if(labels = (const char**) malloc(qty * 8 * sizeof(char))){
			memset(labels,0,qty*8*sizeof(char));
		}
		if(states = (unsigned char*)malloc(qty)){
			memset(states,0,qty);
		}
	}
	
	this->init();
}
*/

Buttongrid::Buttongrid(unsigned char r, unsigned char c, unsigned char cfg){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1);
	columns = c;
	rows = r;
	this->config = cfg;
	
	//Allocate memory for labels and states
	
	if(rows >= 0 && columns >= 0){
		char qty = rows * columns;
		char chars = ((config&BUTTONGRID_NUMLABELS)?1:8);
		if(config&BUTTONGRID_USELABELS){
			if(labels = (const char**) malloc(qty * chars * sizeof(char))){
				memset(labels,0,qty*chars*sizeof(char));
			}
		}
		if(states = (unsigned char*)malloc(qty)){
			memset(states,0,qty);
		}
	}
	this->init();
}

Buttongrid::~Buttongrid(){
	if(text) free(text);
}

void Buttongrid::setSize(int width, int height){
	Widget::setSize(width,height);
	btnWidth = (w-(borderWidth*(columns+1)))/columns;
	btnHeight = (h-(borderWidth*(rows+1)))/rows;
}

void Buttongrid::init(){
	Button::init();
	type = 0x31;
	borderWidth = 2;
	charPos = 0;
}

void Buttongrid::configure(byte size){
	columns = size;
	rows = size;
	gridSize = size;
	show();
}

void Buttongrid::drawGrid(){
	int xPos,yPos,width,height;
	xPos = x;	
	yPos = y;
	width = w;
	height = h;

	//-- outer border
	for(byte i=borderWidth; i!=0;i--){
		Tft.drawRect(xPos,yPos,width,height,borderColor);
		xPos += 1;
		yPos += 1;
		width -= 2;
		height -= 2;
	}
	
	//--nums background rectangle
	Tft.fillRect(x+borderWidth,y+borderWidth,w-2*borderWidth,h-2*borderWidth,bgColor);

	//-- vertical lines
	for(byte j=1;j<columns;j++)
	{
		// draw # of vertical lines depending on borderWidth
		for(byte i=0; i<borderWidth;i++){
			Tft.drawVerticalLine(x + j*(btnWidth + borderWidth) + i,y+borderWidth,h-2*borderWidth,borderColor);
		}	
	}
	
	//-- horizontal lines
	for(byte j=1;j<rows;j++)
	{
		// draw # horizontal lines depending on borderWidth
		for(byte i=0; i<borderWidth;i++){
			Tft.drawHorizontalLine(x+borderWidth, y + j*(btnHeight + borderWidth) + i,w-2*borderWidth,borderColor);
		}
	}
	
	//-- draw contents  
	byte colIndex=0;
	byte rowIndex=0;
	for(byte r=1; r<=rows; r++)
	{
	  for(byte c=1; c<=columns; c++)
	  {
		  drawLabel(getId(r,c));
	  }
	}	
 	return;
}

void Buttongrid::drawLabel(unsigned char id){
	unsigned char colIndex = getColumn(id)-1;
	unsigned char rowIndex = getRow(id)-1;
	unsigned char length;
	int xPos,yPos;
	
	if((config&BUTTONGRID_USELABELS) && (labels[id] != 0)){
		length = getTextLength(labels[id]);
	}else{
		length = Tft.Get_Digits(id);
	}

	xPos = getCenterTextX(x+(colIndex*btnWidth)+borderWidth*(1+colIndex), btnWidth, length);
	yPos = getCenterTextY(y+(rowIndex*btnHeight)+borderWidth*(1+rowIndex), btnHeight);

	if((config&BUTTONGRID_USELABELS) && (labels[id] != 0)){
		Tft.drawString(labels[id],xPos,yPos,fontSize,fgColor);
	}else{
		Tft.drawNumber(id,xPos,yPos,fontSize,fgColor);
	}
	return;
}

void Buttongrid::setEventHandler(void (*functionPointer)(Buttongrid *, unsigned char)){
	eventHandler = functionPointer;
}

unsigned char Buttongrid::getColumn(unsigned char num){
	unsigned char val = num%columns;
	return val==0 ? columns : val;
}

unsigned char Buttongrid::getRow(unsigned char num){
	num -= 1;
	unsigned char mod = num%columns;		
	return num/columns + 1;
}

unsigned char Buttongrid::getId(unsigned char row, unsigned char column){
	unsigned char val = column + (row - 1) * columns;
	return val;
}

void Buttongrid::setLabel(unsigned char id, const char name[8]){
	if((config&BUTTONGRID_USELABELS) && !(config&BUTTONGRID_NUMLABELS))){
		labels[id] = name;
	}		
	return;
}

void Buttongrid::clear(){
	drawGrid();
	lastPressed = 0;
}
 
//Overriden virtual methods
bool Buttongrid::checkTouch(Point* p){
	int boundX1, boundX2, boundY1, boundY2;	
	bool pressed = false;
	if(lastMillis + debounceTime < millis()){ 
		if((p->x > x+borderWidth) && (p->x < x+w-borderWidth) && (p->y > y+borderWidth) && (p->y < y+h-borderWidth)){
			//id coordinates
			for(int r = 1;(r <= rows)&&(!pressed); r++)
			{
				// Determine the bounding y's for this row
				boundY1 = y+(btnHeight)*(r-1) + borderWidth*r;
				boundY2 = boundY1 + btnHeight;
				
				for(int c = 1;(c <= columns)&&(!pressed);c++)
				{
					// Determine the bounding x's for this column
					boundX1 = x + (btnWidth)*(c-1) + borderWidth*c;
					boundX2 = boundX1 + btnWidth;
					
					int id = columns*(r - 1) + c;
					if((p->x > boundX1) && (p->x < boundX2) && (p->y > boundY1) && (p->y < boundY2)){
						
						if(config & BUTTONGRID_MAINTAINED){
							states[id] = (states[id])?0:1;
							if(config&BUTTONGRID_HIGHLIGHT){
								Tft.fillRect(boundX1,boundY1,btnWidth,btnHeight,(states[id])?highlightColor:bgColor);
								drawLabel(id);
							}
						}else{
							if ((config&BUTTONGRID_HIGHLIGHT) && (lastPressed != 0)){
								// Restore last button pressed appearance
								char cIndex = getColumn(lastPressed) - 1;
								char rIndex = getRow(lastPressed) - 1;
								int lastX = x + btnWidth*cIndex + borderWidth*(cIndex + 1);
								int lastY = y + btnHeight*rIndex + borderWidth*(rIndex + 1);
								Tft.fillRect(lastX,lastY,btnWidth,btnHeight,bgColor);
								drawLabel(lastPressed);
								Tft.fillRect(boundX1,boundY1,btnWidth,btnHeight,highlightColor);
								drawLabel(id);
							}
						}
						// Call event handler with pressed button id
						eventHandler(this,id);
						pressed = true;
						lastPressed = id;
					}
				}// -- columns for loop
			}// --rows for loop
		}// -- if touch within widget area
		touched = !touched;
		lastMillis = millis();		
	}// -- debounce
	return true; // <--- False means block further event checking.
}

void Buttongrid::show(){
	drawGrid();
	update();
}

void Buttongrid::update(){
	return;
}
