#include "Display.h"
#include <new>
#include <stdlib.h>
#include <string.h>

namespace {
// Measure with the same text size/font/wrap state used by drawString(). This
// deliberately runs after drawing, so no backend state needs to be changed.
template<class Driver>
auto textBounds(Driver* driver, const char* text, int16_t x, int16_t y,
                unsigned char, unsigned char, int16_t& bx, int16_t& by,
                uint16_t& bw, uint16_t& bh, int)
    -> decltype(driver->getTextBounds(text, x, y, &bx, &by, &bw, &bh), void()) {
    driver->getTextBounds(text, x, y, &bx, &by, &bw, &bh);
}
// Compatibility with older fixed-cell drivers without the GFX bounds API
// (also the fast host operation recorder). Those drivers use cell estimates;
// wrapping/custom-font cleanup requires a backend providing getTextBounds.
template<class Driver>
void textBounds(Driver*, const char*, int16_t x, int16_t y,
                unsigned char length, unsigned char font, int16_t& bx, int16_t& by,
                uint16_t& bw, uint16_t& bh, long) {
    const unsigned char effectiveFont = font ? font : 1;
    bx = x; by = y;
    bw = length * FONT_X * effectiveFont;
    bh = FONT_Y * effectiveFont;
}
}

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
Display::~Display() { free(renderedText); delete contents; }
void Display::init() {
    if (!renderedText && contents && contents->text)
        renderedText = (char*)malloc((size_t)contents->_textSize + 1);
    renderedValid = false;
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
    if (contents) contents->setText(value);
    if (force) render(false);
}
void Display::deleteChar() {
    if (contents && contents->text) {
        size_t used = contents->getTextSize();
        if (used) contents->text[used - 1] = 0;
    }
    update();
}
bool Display::checkTouch(Point*) { return true; }
void Display::show() { visible = true; update(); }
bool Display::matchesRendered() const {
    if (!renderedValid || !renderedText) return false;
    unsigned char length = contents ? contents->getTextSize() : 0;
    return rendered.canvas == myCanvas && rendered.driver == myCanvas->tft &&
        rendered.x == x && rendered.y == y && rendered.w == w && rendered.h == h &&
        rendered.borderWidth == borderWidth && rendered.bg == bgColor &&
        rendered.fg == fgColor && rendered.border == borderColor &&
        rendered.font == fontSize && rendered.length == length &&
        (!length || memcmp(renderedText, contents->text, length) == 0);
}
void Display::rememberRender() {
    unsigned char length = contents ? contents->getTextSize() : 0;
    rendered = {myCanvas, myCanvas->tft, x, y, w, h, borderWidth,
                bgColor, fgColor, borderColor, fontSize, length, 0, 0, 0, 0};
    if (length) {
        // Preserve Widget's origin, including its public fontSize==0 centering.
        // Guitft passes these through uint16_t to the driver's signed cursor.
        const int16_t tx = (uint16_t)getCenterTextX(x, w, length);
        const int16_t ty = (uint16_t)getCenterTextY(y, h);
        textBounds(myCanvas->tft, contents->text, tx, ty, length, fontSize,
                   rendered.textX, rendered.textY, rendered.textW, rendered.textH, 0);
    }
    if (renderedText) {
        if (length) memcpy(renderedText, contents->text, length);
        renderedText[length] = 0;
    }
    renderedValid = true;
}
void Display::render(bool force) {
    if (!canDraw()) return;
    if (!force && matchesRendered()) return;
    const bool sameTarget = renderedValid && rendered.canvas == myCanvas &&
                            rendered.driver == myCanvas->tft;
    const bool moved = sameTarget && (rendered.x != x || rendered.y != y ||
                                     rendered.w != w || rendered.h != h);
    const bool frameChanged = !sameTarget || moved ||
        rendered.borderWidth != borderWidth || rendered.border != borderColor ||
        rendered.bg != bgColor;
    bool clearedOverflow = false;
    if (sameTarget && rendered.length) {
        // The committed footprint includes backend wrapping and effective font
        // metrics. Never reconstruct it using a later style or driver state.
        int tw = rendered.textW, th = rendered.textH;
        int tx = rendered.textX, ty = rendered.textY;
        if (moved || tx < x + borderWidth || ty < y + borderWidth ||
            tx + tw > x + w - borderWidth || ty + th > y + h - borderWidth) {
            myCanvas->tft->fillRect(tx, ty, tw, th, moved ? myCanvas->bgColor : bgColor);
            clearedOverflow = true;
        }
    }
    if (moved && rendered.w > 0 && rendered.h > 0)
        myCanvas->tft->fillRect(rendered.x, rendered.y, rendered.w, rendered.h, myCanvas->bgColor);
    // Fill the whole widget when its border/style changes, including the old border.
    if (frameChanged && w > 0 && h > 0)
        myCanvas->tft->fillRect(x, y, w, h, bgColor);
    else if (w > 2 * borderWidth && h > 2 * borderWidth)
        myCanvas->tft->fillRect(x + borderWidth, y + borderWidth,
                              w - 2 * borderWidth, h - 2 * borderWidth, bgColor);
    if (frameChanged || force || clearedOverflow) drawFrame();
    drawText(fgColor);
    rememberRender();
}
void Display::update() { render(true); }
