#include "Display.h"
#include <new>

Display::Display(unsigned char textLength) {
    myCanvas = NULL;
    x = y = 0;
    contents = new (std::nothrow) Text(textLength);
    init();
}
Display::Display(unsigned int width, unsigned int height, int backgroundColor,
                 int textColor, int borderColor, unsigned char textLength) {
    myCanvas = NULL;
    x = y = 0;
    contents = new (std::nothrow) Text(textLength);
    setSize(width, height);
    setColors(backgroundColor, textColor, borderColor);
    init();
}
Display::~Display() { delete contents; }
void Display::init() {
    type = 0x10;
    borderWidth = 2;
    charPos = 0;
    isButton = false;
}
void Display::drawFrame() {
    if (!canDraw()) return;
    int px = x, py = y, width = w, height = h;
    for (int i = 0; i < borderWidth && width > 0 && height > 0; ++i) {
        myCanvas->tft->drawRect(px++, py++, width, height, borderColor);
        width -= 2; height -= 2;
    }
}
void Display::drawText(int color) {
    if (!canDraw() || !contents || !contents->text || !*contents->text) return;
    int sx = getCenterTextX(x, w, contents->getTextSize());
    myCanvas->tft->drawString(contents->text, sx, getCenterTextY(y, h), fontSize, color);
}
void Display::append(char* value) {
    if (!contents || !contents->text || !value) return;
    size_t used = contents->getTextSize();
    size_t count = 0, space = contents->_textSize - used;
    while (count < space && value[count]) ++count;
    memmove(contents->text + used, value, count);
    contents->text[used + count] = 0;
    update();
}
void Display::append(char value) {
    if (!contents || !contents->text) return;
    size_t used = contents->getTextSize();
    if (used < contents->_textSize) {
        contents->text[used] = value;
        contents->text[used + 1] = 0;
    }
    update();
}
void Display::fitToText() {
    if (contents && contents->getTextSize()) {
        w = contents->getTextSize() * FONT_SPACE * borderWidth + FONT_SPACE;
        h = FONT_Y * borderWidth + FONT_Y;
    }
}
void Display::setText(char* value, bool force) {
    if (force) drawText(bgColor);
    if (contents) contents->setText(value);
    if (force) drawText(fgColor);
}
void Display::deleteChar() {
    if (contents && contents->text) {
        size_t used = contents->getTextSize();
        if (used) contents->text[used - 1] = 0;
    }
    update();
}
bool Display::checkTouch(Point*) { return true; }
void Display::show() { visible = true; drawFrame(); update(); }
void Display::update() {
    if (!canDraw()) return;
    if (w > 2 * borderWidth && h > 2 * borderWidth)
        myCanvas->tft->fillRect(x + borderWidth, y + borderWidth,
                              w - 2 * borderWidth, h - 2 * borderWidth, bgColor);
    drawText(fgColor);
}
