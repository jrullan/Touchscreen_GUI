#include "Buttongrid.h"

Buttongrid::Buttongrid(){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
}

Buttongrid::Buttongrid(unsigned char size){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
	columns = size;
	rows = size;
	gridSize = size;
	
	//Allocate memory for labels
	if(gridSize!=NULL){
		if(labels = (unsigned char**)malloc(gridSize * sizeof(unsigned char*))){
			for(int i=0; i<gridSize; i++)
			{
				if(labels[i] = (unsigned char*)malloc(gridSize)){
					memset(labels[i],0,gridSize);
				}
			}
		}
	}
	
	this->init();
}

Buttongrid::Buttongrid(unsigned char r, unsigned char c){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
	columns = c;
	rows = r;
	//gridSize = size;
	
	//Allocate memory for labels
	if(rows!=NULL){
		if(labels = (unsigned char**)malloc(rows * sizeof(unsigned char*))){
			for(int i=0; i<rows; i++)
			{
				if(labels[i] = (unsigned char*)malloc(columns)){
					memset(labels[i],0,columns);
				}
			}
		}
	}
	
	//Allocate memory for names
	if(rows >= 0 && columns >= 0){
		char qty = rows * columns;
		if(names = (const char**) malloc(qty * 8 * sizeof(char))){
			memset(names,0,qty*8*sizeof(char));
		}
	}
	
	//this->init();

}

Buttongrid::Buttongrid(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor){
	if(text = (char *)malloc(DISPLAY_SIZE+1)) memset(text,0,DISPLAY_SIZE+1); //Had to add one more, to avoid some bug
	x = 0;
	y = 0;
	this->setSize(width,height);
	this->setColors(backgroundColor,textColor,borderColor);
	this->init();
}

Buttongrid::~Buttongrid(){
	if(text) free(text);
}

void Buttongrid::init(){
	Button::init();
	type = 0x31;
	borderWidth = 2;
	charPos = 0;
	
	// Initialize labels with position ID
	for(byte r=1; r<=rows; r++)
	{
		for(byte c=1; c<=columns; c++)
		{
			labels[r-1][c-1] = c+columns*(r-1);
			//Serial.print("Row ");
			//Serial.print(r);
			//Serial.print(" Col ");
			//Serial.print(c);
			//Serial.print(" Num ");
			//Serial.println(c+columns*(r-1));
		}
	}
}

void Buttongrid::drawGrid(){
	int xPos,yPos,width,height;
	int btnWidth,btnHeight;

	xPos = x;	
	yPos = y;
	width = w;
	height = h;
	btnWidth = (w-(borderWidth*(columns+1)))/columns;
	btnHeight = (h-(borderWidth*(rows+1)))/rows;
  
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
		  drawButtonId(getId(r,c),labels[r-1][c-1]);
	  }
	}	
	
	
 	return;
	/*
	//-- Gaps fill
	if(hGap > 0){
		for(byte j=1;j<columns;j++){
			Tft.fillRect(x+j*btnWidth+borderWidth-hGap,y,hGap*2,h,myCanvas->bgColor);
		}
	}
	if(vGap > 0){
		for(byte j=1;j<rows;j++){
			Tft.fillRect(x,y+j*btnHeight+borderWidth-vGap,w,vGap*2,myCanvas->bgColor);
		}
	}  
	*/
}

void Buttongrid::drawButtonId(unsigned char id, uint8_t label){
	//int boundX1, boundX2, boundY1, boundY2;
	int btnWidth = (w-(borderWidth*(columns+1)))/columns;
	int btnHeight = (h-(borderWidth*(rows+1)))/rows;	
	unsigned char colIndex = getColumn(id)-1;
	unsigned char rowIndex = getRow(id)-1;
	unsigned char digits = Tft.Get_Digits(label);
	
	//Calculates position of the text considering
	//its column or row and the btnWidth.
	int xPos = getCenterTextX(x+(colIndex*btnWidth)+borderWidth*(1+colIndex), btnWidth, digits);
	int yPos = getCenterTextY(y+(rowIndex*btnHeight)+borderWidth*(1+rowIndex), btnHeight);

	//Draw contents function
	Tft.drawNumber(label,xPos,yPos,fontSize,BLACK);
}

void Buttongrid::drawLabel(unsigned char id){
	drawButtonId(id,labels[getRow(id)-1][getColumn(id)-1]);
	return;
}

void Buttongrid::setEventHandler(void (*functionPointer)(Buttongrid *, unsigned char)){
	eventHandler = functionPointer;
}

void Buttongrid::configure(byte size){
	columns = size;
	rows = size;
	gridSize = size;
	show();
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

/*
void Buttongrid::setNum(unsigned char id){
	int boundX1, boundX2, boundY1, boundY2;
	int btnWidth = w/columns;
	int btnHeight = h/rows;
	unsigned char colIndex = getColumn(id)-1;
	unsigned char rowIndex = getRow(id)-1;
	unsigned char digits = Tft.Get_Digits(id);
	
	//Calculates initial position of text inside the btnWidth
	//considering the number's width and font size.
	int xPos = btnWidth/2 - (digits*FONT_X*fontSize)/2;//btnWidth/(2) - 6*digits -2;
	int yPos = btnHeight/(2) - 8;
	
	//Calculates position of the text considering
	//its column or row and the btnWidth.
  	xPos = x+(colIndex*btnWidth)+xPos+borderWidth;
  	yPos = y+yPos+(rowIndex*btnHeight);
  	
  	//Draw contents function
  	Tft.drawNumber(id,xPos,yPos,fontSize,BLACK);
}
*/

void Buttongrid::setName(unsigned char id, const char name[8]){
	names[id] = name;	
	return;
}

void Buttongrid::clear(){
	drawGrid();
	lastPressed = 0;
}
 
//Overriden virtual methods

bool Buttongrid::checkTouch(Point* p){
	int boundX1, boundX2, boundY1, boundY2;
	int btnWidth = (w-(borderWidth*(columns+1)))/columns;
	int btnHeight = (h-(borderWidth*(rows+1)))/rows;	
	
	bool pressed = false;
	if(lastMillis + debounceTime < millis()){ 
		if((p->x > x+borderWidth) && (p->x < x+w-borderWidth) && (p->y > y+borderWidth) && (p->y < y+h-borderWidth)){
			//num coordinates
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
					
					int num = columns*(r - 1) + c;
					if((p->x > boundX1) && (p->x < boundX2) && (p->y > boundY1) && (p->y < boundY2)){
						
						// Restore last button pressed appearance
						if(clearLastPressed && lastPressed != 0){
							char cIndex = getColumn(lastPressed) - 1;
							char rIndex = getRow(lastPressed) - 1;
							int lastX = x + btnWidth*cIndex + borderWidth*(cIndex + 1);
							int lastY = y + btnHeight*rIndex + borderWidth*(rIndex + 1);//y+(btnHeight)*(getRow(lastPressed)-1)+borderWidth;
							if(HIGHLIGHT == 1){
								Tft.fillRect(lastX,lastY,btnWidth,btnHeight,bgColor);
							}
							drawLabel(lastPressed);
						}
						
						// Highlight currently pressed button
						if(HIGHLIGHT == 1){
							Tft.fillRect(boundX1,boundY1,btnWidth,btnHeight,highlightColor);
						}
						
						drawLabel(num);
						
						// Call event handler with pressed button id
						eventHandler(this,num);
						pressed = true;
						lastPressed = num;
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
