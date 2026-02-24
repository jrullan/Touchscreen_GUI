#include "Keyboard.h"

// Key layout arrays: 4 rows × 10 columns = 40 chars each
// Row 0 = numbers, Row 1 = top letter row, Row 2 = middle, Row 3 = bottom
// Special keys: '<' = backspace, '^' = shift, '#' = mode toggle
const char Keyboard::keysLower[]   = "1234567890" "qwertyuiop" "asdfghjkl<" "^zxcvbnm.#";
const char Keyboard::keysUpper[]   = "1234567890" "QWERTYUIOP" "ASDFGHJKL<" "^ZXCVBNM.#";
const char Keyboard::keysSpecial[] = "1234567890" "!@#$%^&*()" "-_=+[]{}|<" "~;:'\",/?\\#";

Keyboard::Keyboard(){
	if(text = (char*)malloc(KEYBOARD_BUF_SIZE + 1)) memset(text, 0, KEYBOARD_BUF_SIZE + 1);
	init();
}

Keyboard::Keyboard(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor){
	if(text = (char*)malloc(KEYBOARD_BUF_SIZE + 1)) memset(text, 0, KEYBOARD_BUF_SIZE + 1);
	init();
	setSize(width, height);
	setColors(backgroundColor, textColor, borderColor);
}

Keyboard::~Keyboard(){
	if(text) free(text);
}

void Keyboard::init(){
	Button::init();
	type = 0x33;
	borderWidth = 1;
	textPos = 0;
	debounceTime = 150;
	autoremove = false;
	shiftActive = false;
	specialActive = false;
	targetButton = NULL;
	eventHandler = NULL;
	visible = false;
	setSize(240, 180);
}

int Keyboard::keyW(){
	return w / KEYBOARD_COLS;
}

int Keyboard::keyH(){
	return h / KEYBOARD_ROWS;
}

void Keyboard::append(char c){
	if(textPos < KEYBOARD_BUF_SIZE){
		text[textPos++] = c;
		text[textPos] = 0;
		update();
	}
}

void Keyboard::deleteChar(){
	if(textPos > 0){
		text[--textPos] = 0;
	}
	// Force clear display area when empty
	drawDisplayArea();
}

void Keyboard::clear(){
	while(textPos > 0) text[--textPos] = 0;
	drawDisplayArea();
}

char* Keyboard::getText(){
	return text;
}

byte Keyboard::getTextSize(){
	return textPos;
}

void Keyboard::setEventHandler(void (*functionPointer)(Keyboard*)){
	eventHandler = functionPointer;
}

char Keyboard::getKeyChar(byte row, byte col){
	byte idx = row * KEYBOARD_COLS + col;
	if(specialActive) return keysSpecial[idx];
	if(shiftActive)   return keysUpper[idx];
	return keysLower[idx];
}

void Keyboard::drawFrame(){
	int kw = keyW();
	int kh = keyH();

	// Background
	myCanvas->tft->fillRect(x, y, w, h, bgColor);

	// Display area background (row 0)
	int dispW = kw * 9; // 9 columns for text
	myCanvas->tft->fillRect(x, y, dispW, kh, BLACK);
	// DEL button background (col 9)
	myCanvas->tft->fillRect(x + dispW, y, kw, kh, bgColor);

	// Outer border
	int xPos = x, yPos = y, bw = w, bh = h;
	for(byte i = borderWidth; i != 0; i--){
		myCanvas->tft->drawRect(xPos++, yPos++, bw, bh, borderColor);
		bw -= 2;
		bh -= 2;
	}

	// Horizontal lines between rows
	for(byte r = 1; r < KEYBOARD_ROWS; r++){
		int ly = y + r * kh;
		for(byte i = 0; i < borderWidth; i++){
			myCanvas->tft->drawHorizontalLine(x, ly + i, w, borderColor);
		}
	}

	// Vertical lines for rows 1-4 (char key rows)
	for(byte r = 1; r <= 4; r++){
		int ry = y + r * kh;
		for(byte c = 1; c < KEYBOARD_COLS; c++){
			int lx = x + c * kw;
			for(byte i = 0; i < borderWidth; i++){
				myCanvas->tft->drawVerticalLine(lx + i, ry, kh, borderColor);
			}
		}
	}

	// Vertical line between display and DEL (row 0)
	for(byte i = 0; i < borderWidth; i++){
		myCanvas->tft->drawVerticalLine(x + dispW + i, y, kh, borderColor);
	}

	// Vertical line between SPACE and ENTER (row 5)
	int row5y = y + 5 * kh;
	int splitX = x + 6 * kw;
	for(byte i = 0; i < borderWidth; i++){
		myCanvas->tft->drawVerticalLine(splitX + i, row5y, kh, borderColor);
	}

	// Draw key labels
	drawKeys();

	// DEL label
	int dx = getCenterTextX(x + dispW, kw, 2);
	int dy = getCenterTextY(y, kh);
	myCanvas->tft->drawString("<-", dx, dy, fontSize, fgColor);

	// SPACE label
	int sx = getCenterTextX(x, 6 * kw, 3);
	int sy = getCenterTextY(y + 5 * kh, kh);
	myCanvas->tft->drawString("SPC", sx, sy, fontSize, fgColor);

	// ENTER label
	int ex = getCenterTextX(x + 6 * kw, 4 * kw, 2);
	int ey = getCenterTextY(y + 5 * kh, kh);
	myCanvas->tft->drawString("OK", ex, ey, fontSize, fgColor);
}

void Keyboard::drawKeys(){
	int kw = keyW();
	int kh = keyH();
	char buf[2] = {0, 0};

	for(byte r = 0; r < 4; r++){
		int ry = y + (r + 1) * kh; // rows 1-4 on screen
		for(byte c = 0; c < KEYBOARD_COLS; c++){
			char ch = getKeyChar(r, c);
			int cx = x + c * kw;

			// Clear key background
			myCanvas->tft->fillRect(cx + borderWidth, ry + borderWidth,
				kw - borderWidth, kh - borderWidth, bgColor);

			// Draw label
			if(ch == '<'){
				int lx = getCenterTextX(cx, kw, 2);
				int ly = getCenterTextY(ry, kh);
				myCanvas->tft->drawString("<-", lx, ly, fontSize, fgColor);
			} else if(ch == '^'){
				int lx = getCenterTextX(cx, kw, 2);
				int ly = getCenterTextY(ry, kh);
				uint16_t color = shiftActive ? highlightColor : fgColor;
				if(!highlightColor) color = fgColor;
				myCanvas->tft->drawString("Sh", lx, ly, fontSize, color);
			} else if(ch == '#'){
				int lx = getCenterTextX(cx, kw, 2);
				int ly = getCenterTextY(ry, kh);
				uint16_t color = specialActive ? highlightColor : fgColor;
				if(!highlightColor) color = fgColor;
				myCanvas->tft->drawString("!#", lx, ly, fontSize, color);
			} else {
				buf[0] = ch;
				int lx = getCenterTextX(cx, kw, 1);
				int ly = getCenterTextY(ry, kh);
				myCanvas->tft->drawString(buf, lx, ly, fontSize, fgColor);
			}
		}
	}
}

void Keyboard::drawDisplayArea(){
	int kw = keyW();
	int kh = keyH();
	int dispW = kw * 9;

	// Clear display area
	myCanvas->tft->fillRect(x + borderWidth, y + borderWidth,
		dispW - 2 * borderWidth, kh - 2 * borderWidth, BLACK);

	// Draw text if any
	if(textPos > 0){
		// Show as many trailing characters as fit in the display
		int maxChars = (dispW - 4) / (FONT_X * fontSize);
		char* displayText = text;
		if(textPos > maxChars){
			displayText = text + (textPos - maxChars);
		}
		int tx = x + borderWidth + 2;
		int ty = getCenterTextY(y, kh);
		myCanvas->tft->drawString(displayText, tx, ty, fontSize, GREEN);
	}
}

bool Keyboard::checkTouch(Point* p){
	if(!eventHandler && !targetButton) return false;

	int kw = keyW();
	int kh = keyH();

	if(lastMillis + debounceTime < millis()){
		if((p->x > x) && (p->x < x + w) && (p->y > y) && (p->y < y + h)){

			// Determine row and column
			int col = (p->x - x) / kw;
			int row = (p->y - y) / kh;

			if(col >= KEYBOARD_COLS) col = KEYBOARD_COLS - 1;
			if(row >= KEYBOARD_ROWS) row = KEYBOARD_ROWS - 1;

			if(row == 0){
				// Display row: only DEL button (col 9)
				if(col >= 9){
					deleteChar();
				}
			} else if(row >= 1 && row <= 4){
				// Character rows (mapped to key array rows 0-3)
				char ch = getKeyChar(row - 1, col);
				if(ch == '<'){
					deleteChar();
				} else if(ch == '^'){
					shiftActive = !shiftActive;
					if(shiftActive) specialActive = false;
					drawKeys();
				} else if(ch == '#'){
					specialActive = !specialActive;
					if(specialActive) shiftActive = false;
					drawKeys();
				} else {
					append(ch);
				}
			} else if(row == 5){
				// Bottom row: SPACE (cols 0-5) or ENTER (cols 6-9)
				if(col < 6){
					append(' ');
				} else {
					// ENTER pressed
					if(autoremove){
						if(myCanvas->widgets.peek() == this){
							myCanvas->pop();
						}
					}
					if(eventHandler) eventHandler(this);
				}
			}

			lastMillis = millis();
		}
	}
	return false;
}

void Keyboard::show(){
	drawFrame();
	update();
	if(!visible) visible = true;
}

void Keyboard::show(Button* btn){
	targetButton = btn;
	show();
}

void Keyboard::hide(){
	visible = false;
	myCanvas->redrawRegion(x, y, w, h);
}

void Keyboard::reset(){
	clear();
	shiftActive = false;
	specialActive = false;
}

void Keyboard::update(){
	drawDisplayArea();
}
