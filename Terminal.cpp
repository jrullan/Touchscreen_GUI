 #include "Terminal.h"
 
Terminal::Terminal(int width, int height, uint8_t dir, int fontSize){
	this->w = width;
	this->h = height;
	this->fontSize = fontSize;
	
	bgColor = BLACK;
	fgColor = GRAY1;
	borderColor = GRAY1;
	
	direction = dir;
	keepColors = true;
	borderWidth = 1;
	horizontalBleed = horizontalBleed * fontSize*FONT_X;
	verticalBleed = verticalBleed * fontSize*FONT_Y / 2;
	lineSpace = fontSize*FONT_Y / 2;	
	lines  = (this->h-2*borderWidth-2*verticalBleed)/(fontSize*FONT_Y+lineSpace);
	lines = (lines > MAX_LINES) ? MAX_LINES : lines;
	memset(linesBuffer, 0, MAX_LINES * sizeof(char*));
	
	// Calculate characters based on fontSize and width
	maxCharacters = (this->w - 2*borderWidth - 2*horizontalBleed)/(fontSize*FONT_X);
	// Allocate memory for characters
	for(int i = 0; i < lines; i++){
		if(linesBuffer[i] = (char*) malloc((maxCharacters+1) * sizeof(char))){
				memset(linesBuffer[i],0,(maxCharacters+1)*sizeof(char));
		}
	}

}
 
Terminal::~Terminal(){}
 
/*
 * Main print function.
 * Prints a char array, given by a pointer with the highlight color
 * specified. 
 */
void Terminal::print(char* string,uint16_t highColor){
	highlightColor = highColor;
	int length = Widget::getTextLength(string);
	length = (length > maxCharacters) ? maxCharacters : length;
	char lineIndex = (direction == TERMINAL_SCROLL_DOWN) ? 0 : lines - 1;
	
	// Only scroll if:
	if(direction==TERMINAL_SCROLL_UP && linesIndex <= lines -1){
		lineIndex = linesIndex++;
	}else{
		scroll();
	}
	
	linesColors[lineIndex] = (highlightColor == NULL) ? fgColor : highlightColor;
	
	for(int i=0; i<length; i++){
		linesBuffer[lineIndex][i] = string[i];
	}
	linesBuffer[lineIndex][length] = 0;
	
	update();
}

/*
 * Implementation of print("some %d characters",4)
 * similar to C++ printf function. Only %d is recognized here.
 * First searches for the position of the % symbol in the string.
 * Then if the next character is d then convert the number to 
 * a character array in num_string using Widget's convert_str
 * and copy them into the new_string in the inverted order. 
 * Then update the indexes and keep copying the rest of the characters
 * into the new_string array. 
 */
void Terminal::printf(char* string, int num, uint16_t highColor){
	highlightColor = highColor;
	char lineIndex = (direction == TERMINAL_SCROLL_DOWN) ? 0 : lines - 1;
	
	// Only scroll up when it is already in the last line:
	if(direction==TERMINAL_SCROLL_UP && linesIndex <= lines -1){
		lineIndex = linesIndex++;
	}else{
		scroll();
	}
	
	linesColors[lineIndex] = (highlightColor == NULL) ? fgColor : highlightColor;
	

	// Here is the code when the characters are copied to the terminal buffer
	uint8_t str_size = Widget::getTextLength(string);
	uint8_t num_size = Widget::getIntLength(num);
	char temp;
	char* new_string = linesBuffer[lineIndex];

	uint8_t j = 0;
	for(uint8_t i=0; i<str_size; i++){

		// Scan for the %d placeholder in the provided string
		if(string[i] == '%' && string[i+1] == 'd'){
			
			//Convert the number to characters and put them in the
			//new_string starting at the current position (i)
			Widget::convert_str(num,&new_string[i]);			
			j += num_size-1;
			i++;
		}else{
			new_string[j] = string[i];			
		}
		// Exit if at the end of the terminal buffer
		if(j>=maxCharacters-1) break;
		j++;
	}
	new_string[j] = 0;

	update();
}


/*
 * Calls the corresponding scroll function based on the
 * direction property.
 */
void Terminal::scroll(){
	myCanvas->tft->fillRect(this->x+borderWidth,this->y+borderWidth,this->w-2*borderWidth,this->h-2*borderWidth,this->bgColor);

	if(direction){
		scrollDown();
	}else{
		scrollUp();
	}
}

void Terminal::scrollDown(){
	// Scroll Down
	for(int line = lines-1; line!=0; line--){
		for(int i=0; i<maxCharacters; i++){
			linesBuffer[line][i] = linesBuffer[line-1][i];
		}
		linesColors[line] = linesColors[line-1];
	}
}

void Terminal::scrollUp(){
	// Scroll Up
	for(int line = 0; line<lines-1; line++){
		for(int i=0; i<maxCharacters; i++){
			linesBuffer[line][i] = linesBuffer[line+1][i];
		}
		linesColors[line] = linesColors[line+1];
	}
}

	/*
	* Clears the terminal and all lines text are cleared
	*/
void Terminal::clear(){
	for(int i=0;i<lines;i++){
		linesBuffer[i][0]=0;
	}
	linesIndex = 0;
	myCanvas->tft->fillRect(this->x+borderWidth,this->y+borderWidth,this->w-2*borderWidth,this->h-2*borderWidth,this->bgColor);
}

void Terminal::drawFrame(){
  int xPos = x;	
  int width = w;
  int yPos = y;
  int height = h;
  for(int i=borderWidth; i!=0;i--){
    myCanvas->tft->drawRect(xPos++,yPos++,width--,height--,borderColor);
    width--;
    height--;
  }
}

void Terminal::show(){
	drawFrame();
	myCanvas->tft->fillRect(this->x+borderWidth,this->y+borderWidth,this->w-2*borderWidth,this->h-2*borderWidth,this->bgColor);
	update();
}

void Terminal::update(){
	uint16_t color;
	
	//Calculate position for first line
	int lineX = this->x + borderWidth + horizontalBleed;
	int lineY = this->y + borderWidth + verticalBleed;

	char lineIndex = (direction) ? 0 : lines - 1;
	
	for(int i=0; i<lines; i++){
		if(i==lineIndex){
			color = linesColors[i];
		}else{
			color = (keepColors)?linesColors[i]:fgColor;
		}
		myCanvas->tft->drawString(linesBuffer[i], lineX, lineY+(i*(fontSize*FONT_Y+lineSpace)), this->fontSize, color);//color);
	}
}
