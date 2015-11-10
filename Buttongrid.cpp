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
	borderWidth = 2;
	charPos = 0;
	
	// Initialize labels with position ID
	for(byte r=1; r<=gridSize; r++)
	{
		for(byte c=1; c<=gridSize; c++)
		{
			labels[r-1][c-1] = c+gridSize*(r-1);
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
  btnWidth = w / columns;
  btnHeight = h / rows;
  
  //--nums background rectangle
  Tft.fillRectangle(xPos,yPos,width,height,bgColor);
	//-- outer border
  for(byte i=borderWidth; i!=0;i--){
    Tft.drawRectangle(xPos++,yPos++,width--,height--,borderColor);
    width--;
    height--;
  }
  
  //-- horizontal lines
  for(byte j=0;j<rows;j++)
  {
	// draw # horizontal lines depending on borderWidth
  	for(byte i=0; i<borderWidth;i++){
    	Tft.drawHorizontalLine(x,y+btnHeight*j+i,width+borderWidth,borderColor);
  	}
  }
  
  //-- vertical lines
  for(byte j=0;j<columns;j++)
  {
	// draw # of vertical lines depending on borderWidth
  	for(byte i=0; i<borderWidth;i++){
    	Tft.drawVerticalLine(x+btnWidth*j+i,y+borderWidth,height+borderWidth,borderColor);
  	}
  }

  //-- draw contents  

  byte colIndex=0;
  byte rowIndex=0;
  /*
  for(byte number=1; number<=(columns*rows); number++)
  {
	  //setNum(number);
  }*/
  //Serial.println("Setting Labels=======");
  for(byte i=1; i<=rows; i++)
  {
	  for(byte j=1; j<=columns; j++)
	  {
		  byte num = j+rows*(i-1);
		  
		  //Serial.print("num: ");
		  //Serial.print(num);
		  //Serial.print(" label: ");
		  //Serial.print(labels[i-1][j-1]);
		  //Serial.print(",");
		  
		  setLabel(num,labels[i-1][j-1]);
	  }
	  //Serial.println();
  }
  
}

void Buttongrid::setEventHandler(void (*functionPointer)(Buttongrid *, unsigned char)){
	eventHandler = functionPointer;
}

void Buttongrid::configure(byte size, byte f){
	columns = size;
	rows = size;
	gridSize = size;
	font_size = f;
	show();
}

unsigned char Buttongrid::getColumn(unsigned char num){
	unsigned char val = num%columns;
	return val==0 ? columns : val;
}

unsigned char Buttongrid::getRow(unsigned char num){
	unsigned char val = num%rows;
	return val==0 ? num/rows : (num-val)/rows+1;	
}

unsigned char Buttongrid::getNumber(unsigned char row, unsigned char column){
	unsigned char val = (row - 1) * columns + column;
	return val;
}

void Buttongrid::setNum(unsigned char num){
	int boundX1, boundX2, boundY1, boundY2;
	int btnWidth = w/columns;
	int btnHeight = h/rows;
	unsigned char colIndex = getColumn(num)-1;
	unsigned char rowIndex = getRow(num)-1;
	unsigned char digits = Tft.Get_Digits(num);
	
	//Calculates initial position of text inside the btnWidth
	//considering the number's width and font size.
	int xPos = btnWidth/2 - (digits*FONT_X*font_size)/2;//btnWidth/(2) - 6*digits -2;
	int yPos = btnHeight/(2) - 8;
	
	//Calculates position of the text considering
	//its column or row and the btnWidth.
  	xPos = x+(colIndex*btnWidth)+xPos+borderWidth;
  	yPos = y+yPos+(rowIndex*btnHeight);
  	
  	//Draw contents function
  	Tft.drawNumber(num,xPos,yPos,font_size,BLACK);
}

void Buttongrid::setLabel(unsigned char num, unsigned char label){
	int boundX1, boundX2, boundY1, boundY2;
	int btnWidth = w/columns;
	int btnHeight = h/rows;
	unsigned char colIndex = getColumn(num)-1;
	unsigned char rowIndex = getRow(num)-1;
	unsigned char digits = Tft.Get_Digits(label);
	
	//Calculates initial position of text inside the btnWidth
	//considering the number's width and font size.
	int xPos = btnWidth/2 - (digits*FONT_X*font_size)/2;//btnWidth/(2) - 6*digits -2;
	int yPos = btnHeight/(2) - 8;
	
	//Calculates position of the text considering
	//its column or row and the btnWidth.
  	xPos = x+(colIndex*btnWidth)+xPos+borderWidth;
  	yPos = y+yPos+(rowIndex*btnHeight);
  	
  	//Draw contents function
  	Tft.drawNumber(label,xPos,yPos,font_size,BLACK);	
}

void Buttongrid::clear(){
	drawGrid();
	lastPressed = 0;
}
 
//Overriden virtual methods

bool Buttongrid::checkTouch(Point* p){
	int boundX1, boundX2, boundY1, boundY2;
	int btnWidth = w/columns;
	int btnHeight = h/rows;
	bool pressed = false;
	if(lastMillis + debounceTime < millis()){ 
		if((p->x > x+borderWidth) && (p->x < x+w-borderWidth) && (p->y > y+borderWidth) && (p->y < y+h-borderWidth)){
			//num coordinates
			for(int r = 1;(r <= rows)&&(!pressed); r++)
			{
				// Determine the bounding y's for this row
				boundY1 = y+(btnHeight)*(r-1)+borderWidth;
				boundY2 = y+(btnHeight)*r-borderWidth;
				for(int c = 1;(c <= columns)&&(!pressed);c++)
				{
					// Determine the bounding x's for this column
					boundX1 = x + (btnWidth)*(c-1) + borderWidth;
					boundX2 = x + (btnWidth)*c - borderWidth;
					int num = columns*(r - 1) + c;
					if((p->x > boundX1) && (p->x < boundX2) && (p->y > boundY1) && (p->y < boundY2)){
						
						// Restore last button pressed appearance
						if(lastPressed != 0){
							int lastX = x + (btnWidth)*(getColumn(lastPressed)-1) + borderWidth;
							int lastY = y+(btnHeight)*(getRow(lastPressed)-1)+borderWidth;
							//Tft.fillRectangle(lastX,lastY,btnWidth-borderWidth-1,btnHeight-borderWidth-1,bgColor);
							setLabel(lastPressed,labels[getRow(lastPressed)-1][getColumn(lastPressed)-1]);
						}
						
						// Highlight currently pressed button
						Tft.fillRectangle(boundX1,boundY1,btnWidth-borderWidth-1,btnHeight-borderWidth-1,YELLOW);
						//setNum(num);
						setLabel(num,labels[r-1][c-1]);
						
						
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

bool Buttongrid::isButton(){
	return true;
}

void Buttongrid::show(){
	drawGrid();
	update();
}

void Buttongrid::update(){
	return;
  /*	
  int btnWidth = w / columns;
  int btnHeight = h / rows;
  //Tft.fillRectangle(x+borderWidth,y+borderWidth,(btnWidth*2)-borderWidth,btnHeight-borderWidth*2,BLACK);
  //Count characters to center on the button - Nice trick from the Tft2 library
  if(*text){
    char* chars = text;
    int size = 0;
    while(*chars){
      *chars++;
      size++;
    }
    //Calculate centered position of the text
    int stringX = x+(btnWidth*2-size*FONT_SPACE*borderWidth)/2;
    int stringY = y+(btnHeight-FONT_Y*borderWidth)/2;
    //Tft.drawString(text,stringX,stringY,borderWidth,GREEN);
  }	
  */
}
