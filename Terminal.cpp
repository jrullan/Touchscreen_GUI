 #include "Terminal.h"
 
Terminal::Terminal(int width, int height, uint8_t dir, int fontSize){
	this->w = width;
	this->h = height;
	this->fontSize = fontSize;
	direction = dir;
	borderWidth = 1;
	horizontalBleed = horizontalBleed * fontSize*FONT_X;
	verticalBleed = verticalBleed * fontSize*FONT_Y / 2;
	lineSpace = fontSize*FONT_Y / 2;	
	lines  = (this->h-2*borderWidth-2*verticalBleed)/(fontSize*FONT_Y+lineSpace);
	lines = (lines > MAX_LINES) ? MAX_LINES : lines;
	memset(linesBuffer, 0, MAX_LINES * sizeof(char*));
	
	// Allocate memory for lines
	//if(linesBuffer = (char**)malloc(this->lines)){
		//memset(linesBuffer,0,sizeof(char*)*this->lines);
	//}
	
	
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
 
void Terminal::print(char* string){
	int length = Widget::getTextLength(string);
	length = (length > maxCharacters) ? maxCharacters : length;
	
	scroll();
	
	char lineIndex = (direction) ? 0 : lines - 1;
	for(int i=0; i<length; i++){
		linesBuffer[lineIndex][i] = string[i];
	}
	linesBuffer[lineIndex][length] = 0;
	
	update();
}

void Terminal::print(char* string, int num){
	char numChar[DISPLAY_SIZE];
	char numStr[DISPLAY_SIZE];
	char chars = 0;
	while(num >= 10)
	{
		numChar[chars++] = num%10;
		num /= 10;
	}
	numChar[chars++] = num;
	for(int j = 0; j < chars; j++)
	{
		numStr[chars-1-j] = '0'+numChar[j];
	}
	numStr[chars]=0;	
	
	int numLength = Widget::getTextLength(numStr);
	int strLength = Widget::getTextLength(string);
	int length = strLength + numLength;
	length = (length > maxCharacters) ? maxCharacters : length;
	
	scroll();
	
	char lineIndex = (direction) ? 0 : lines - 1;
	for(int i=0; i<length; i++){
		if(i >= strLength){
			linesBuffer[lineIndex][i] = numStr[i-strLength];
		}else{
			linesBuffer[lineIndex][i] = string[i];
		}
	}
	linesBuffer[lineIndex][length] = 0;
	
	update();	
}

void Terminal::print(int num){
	char numChar[DISPLAY_SIZE];
	char chars = 0;
	
	// Extract characters representing the powers of ten
	while(num >= 10)
	{
		numChar[chars++] = num%10;
		num /= 10;
	}
	numChar[chars++] = num;
	
	scroll();
	
	char lineIndex = (direction) ? 0 : lines - 1;
	for(int j = 0; j < chars; j++)
	{
		linesBuffer[lineIndex][chars-1-j] = '0'+numChar[j];
	}
	linesBuffer[lineIndex][chars]=0;
	
	update();
}


void Terminal::scroll(){
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
	}
}

void Terminal::scrollUp(){
	// Scroll Up
	for(int line = 0; line<lines-1; line++){
		for(int i=0; i<maxCharacters; i++){
			linesBuffer[line][i] = linesBuffer[line+1][i];
		}
	}
}

void Terminal::clear(){
	Tft.fillRect(this->x+borderWidth,this->y+borderWidth,this->w-2*borderWidth,this->h-2*borderWidth,this->bgColor);
}

void Terminal::drawFrame(){
  int xPos = x;	
  int width = w;
  int yPos = y;
  int height = h;
  for(int i=borderWidth; i!=0;i--){
    Tft.drawRect(xPos++,yPos++,width--,height--,borderColor);
    width--;
    height--;
  }
}

void Terminal::show(){
	drawFrame();
	update();
}

void Terminal::update(){
	clear();

	//Calculate position for first line
	int lineX = this->x + borderWidth + horizontalBleed;
	int lineY = this->y + borderWidth + verticalBleed;

	for(int i=0; i<lines; i++){
		Tft.drawString(linesBuffer[i], lineX, lineY+(i*(fontSize*FONT_Y+lineSpace)), this->fontSize, this->fgColor);
	}
}
