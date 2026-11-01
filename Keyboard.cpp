#include "Keyboard.h"
#include <climits>
#include "Screen.h"

namespace {
// Own only GFX's lightweight drawing state, not a framebuffer or the TFT.
// A fresh GFX instance uses the classic font; the shared TFT may use any font.
class KeyboardGlyphs : public Adafruit_GFX {
public:
    explicit KeyboardGlyphs(Guitft& tft)
        : Adafruit_GFX(tft.width(), tft.height()), tft_(tft) {}
    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        tft_.drawPixel(x, y, color);
    }
    void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                       uint16_t color) override {
        tft_.fillRect(x, y, w, h, color);
    }
private:
    Guitft& tft_;
};
}

Keyboard::Keyboard() : Button() { init(); }
Keyboard::Keyboard(unsigned int width, unsigned int height, int backgroundColor,
                   int textColor, int borderColor) : Button() {
    init();
    w = width <= INT_MAX ? static_cast<int>(width) : 0;
    h = height <= INT_MAX ? static_cast<int>(height) : 0;
    setColors(backgroundColor, textColor, borderColor);
}
void Keyboard::init() {
    type = 0x36;
    w = 240; h = 192; fontSize = 1;
    length_ = 0; buffer_[0] = 0;
    autoremove = shiftActive = specialActive = false;
    targetButton = nullptr; result_ = Editing; sessionValid_ = true;
    handler_ = nullptr;
}
bool Keyboard::setText(const char* text) {
    unsigned int size = 0;
    if (text) {
        while (size <= KEYBOARD_BUF_SIZE && text[size]) {
            const unsigned char c = static_cast<unsigned char>(text[size]);
            if (c < 32 || c > 126) return false;
            ++size;
        }
        if (size > KEYBOARD_BUF_SIZE) return false;
    }
    if (size) memmove(buffer_, text, size);
    buffer_[size] = 0;
    length_ = static_cast<uint8_t>(size);
    return true;
}
void Keyboard::append(char c) {
    if (static_cast<unsigned char>(c) < 32 || static_cast<unsigned char>(c) > 126 ||
        length_ >= KEYBOARD_BUF_SIZE) return;
    buffer_[length_++] = c; buffer_[length_] = 0;
}
void Keyboard::deleteChar() { if (length_) buffer_[--length_] = 0; }
void Keyboard::clear() { length_ = 0; buffer_[0] = 0; }
void Keyboard::reset() {
    clear(); shiftActive = specialActive = false;
    targetButton = nullptr; result_ = Editing; sessionValid_ = true;
}

Keyboard::Cell Keyboard::cell(unsigned int row, unsigned int col) const {
    if (row >= 6 || col >= 10) return {Action::None, 0, ""};
    if (row == 0) return col == 9 ? Cell{Action::Backspace, 0, "<"} : Cell{Action::None, 0, ""};
    if (row == 5) {
        if (col < 6) return {Action::Space, 0, "Space"};
        return col < 8 ? Cell{Action::Cancel, 0, "Cancel"} : Cell{Action::Accept, 0, "OK"};
    }
    if (row == 4 && col == 0) return {Action::Shift, 0, "^"};
    if (row == 4 && col == 9) return {Action::Special, 0, "#"};
    static const char lower[4][11] = {"qwertyuiop", "asdfghjkl`", "zxcvbnm,./", " -_=~?!:; "};
    static const char symbols[4][11] = {"1234567890", "!@#$%^&*()", "-_=+[]{}\\|", " ;:'\",.<> "};
    char c = specialActive ? symbols[row-1][col] : lower[row-1][col];
    if (!specialActive && shiftActive && c >= 'a' && c <= 'z') c -= 'a'-'A';
    return {Action::Character, c, nullptr};
}
bool Keyboard::layoutValid() const {
    // Cancel occupies two cells and needs six actual 6px font advances.
    if (!sessionValid_ || !canDraw() || !fontSize || w <= 0 || h <= 0 || borderWidth < 0) return false;
    const int64_t cw = w / 10, ch = h / 6, b = borderWidth, f = fontSize;
    return cw >= 18*f + 2*b && ch >= 8*f + 2*b &&
           myCanvas->tft->width() > 0 && myCanvas->tft->height() > 0;
}
void Keyboard::paintRect(int64_t left, int64_t top, int64_t width, int64_t height, int color) {
    const int64_t right = left + width, bottom = top + height;
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    const int64_t r = right < myCanvas->tft->width() ? right : myCanvas->tft->width();
    const int64_t b = bottom < myCanvas->tft->height() ? bottom : myCanvas->tft->height();
    if (r > left && b > top) myCanvas->tft->fillRect(left, top, r-left, b-top, color);
}
void Keyboard::paintCell(unsigned int row, unsigned int col, unsigned int span, const char* label,
                         bool active, bool frame) {
    const int64_t left = int64_t(x) + int64_t(w)*col/10;
    const int64_t top = int64_t(y) + int64_t(h)*row/6;
    const int64_t width = int64_t(w)*(col+span)/10 - int64_t(w)*col/10;
    const int64_t height = int64_t(h)*(row+1)/6 - int64_t(h)*row/6;
    if (frame) paintRect(left, top, width, height, borderColor);
    paintRect(left+borderWidth, top+borderWidth, width-2LL*borderWidth, height-2LL*borderWidth,
              active ? highlightColor : bgColor);
    const int64_t tw = strlen(label)*6LL*fontSize, th = 8LL*fontSize;
    const int64_t tx = left+(width-tw)/2, ty = top+(height-th)/2;
    // Skip partial glyphs instead of relying on GFX wrapping or narrowing coordinates.
    if (tw && tw <= width-2LL*borderWidth && th <= height-2LL*borderWidth &&
        tx >= 0 && ty >= 0 && tx+tw <= myCanvas->tft->width() && ty+th <= myCanvas->tft->height()) {
        KeyboardGlyphs glyphs(*myCanvas->tft);
        for (unsigned int i = 0; label[i]; ++i)
            glyphs.drawChar(tx + i*6LL*fontSize, ty,
                            static_cast<unsigned char>(label[i]), fgColor, fgColor, fontSize);
    }
}
void Keyboard::paintTextField(bool frame) {
    if (!layoutValid()) return;
    char display[KEYBOARD_BUF_SIZE+1];
    unsigned int fit = (int64_t(w)*9/10-2LL*borderWidth)/(6LL*fontSize);
    if (fit > length_) fit = length_;
    memcpy(display, buffer_+length_-fit, fit); display[fit] = 0;
    // Clear the whole interior before centering the new tail, including on deletion.
    paintCell(0,0,9,display,false,frame);
}
void Keyboard::drawFrame() {
    if (!layoutValid()) return;
    paintTextField(true);
    paintCell(0,9,1,"<");
    for (unsigned int row=1; row<=4; ++row) for (unsigned int col=0; col<10; ++col) {
        const Cell key = cell(row,col);
        char glyph[2] = {key.character,0};
        paintCell(row,col,1,key.label ? key.label : glyph,
                  (key.action == Action::Shift && shiftActive) || (key.action == Action::Special && specialActive));
    }
    paintCell(5,0,6,"Space"); paintCell(5,6,2,"Cancel"); paintCell(5,8,2,"OK");
}
void Keyboard::show() {
    if (!sessionValid_) return;
    result_ = Editing;
    visible = true;
    drawFrame();
}
void Keyboard::show(Button* target) {
    // Never call strlen on borrowed mutable Text storage: validate within capacity+1.
    bool valid = target && target != this && target->contents && target->contents->text;
    unsigned int size = 0;
    if (valid) {
        const Text* text = target->contents;
        while (size <= text->_textSize && text->text[size]) ++size;
        valid = size <= text->_textSize && size <= KEYBOARD_BUF_SIZE;
        if (valid) valid = setText(text->text);
    }
    if (!valid) {
        hide(); targetButton = nullptr; sessionValid_ = false; result_ = Editing;
        return;
    }
    targetButton = target; sessionValid_ = true; result_ = Editing;
    shiftActive = specialActive = false;
    show();
}
void Keyboard::restoreBackground(Canvas* canvas) {
    // Screen::show forcibly re-shows hidden children; do not call Canvas::redraw
    // here, or a Screen-registered keyboard would resurrect itself.
    paintRect(0,0,canvas->tft->width(),canvas->tft->height(),canvas->bgColor);
    Screen* screen = canvas->currentScreen;
    if (screen) {
        paintRect(screen->x,screen->y,screen->w,screen->h,screen->bgColor);
        for (int i=0; i<screen->widgets.count(); ++i) {
            Widget* widget = screen->widgets[i];
            if (widget && widget != this && widget->visible) widget->show();
        }
    }
    canvas->showWidgets();
}
void Keyboard::hide() {
    const bool restore = layoutValid();
    visible = false;
    if (restore) restoreBackground(myCanvas);
}
void Keyboard::setEventHandler(void (*handler)(Keyboard*)) { handler_ = handler; }
void Keyboard::finish(Result result) {
    result_ = result;
    targetButton = nullptr;
    Canvas* canvas = myCanvas;
    hide();
    if (autoremove && canvas) {
        if (!canvas->remove(this) && canvas->currentScreen &&
            canvas->currentScreen->widgets.contains(this)) canvas->currentScreen->remove(this);
    }
    void (*callback)(Keyboard*) = handler_;
    if (callback) callback(this); // Last action: callback may delete this.
}
bool Keyboard::accept() {
    if (!sessionValid_ || result_ != Editing) return false;
    if (targetButton) {
        Text* text = targetButton->contents;
        if (!text || !text->text || length_ > text->_textSize) return false;
        // Existing Text capacity was checked first. Its setter uses memmove, not allocation.
        text->setText(buffer_);
    }
    finish(Accepted);
    return true;
}
void Keyboard::cancel() {
    if (!sessionValid_ || result_ != Editing) return;
    finish(Cancelled);
}
void Keyboard::update() { drawFrame(); }
bool Keyboard::checkTouch(Point* p) {
    if (!p || !layoutValid()) return true;
    const int64_t dx = int64_t(p->x)-x, dy = int64_t(p->y)-y;
    if (dx < 0 || dy < 0 || dx >= w || dy >= h || p->x < 0 || p->y < 0 ||
        p->x >= myCanvas->tft->width() || p->y >= myCanvas->tft->height()) return true;
    const unsigned long now = millis();
    if (static_cast<unsigned long>(now-lastMillis) < debounceTime) return false;
    lastMillis = now;
    // Inverse of floor(i*extent/count): remainder pixels belong to the cell drawn there.
    const unsigned int col = ((dx+1)*10-1)/w, row = ((dy+1)*6-1)/h;
    const Cell key = cell(row,col);
    const uint8_t previousLength = length_;
    switch (key.action) {
    case Action::Character: append(key.character); break;
    case Action::Backspace: deleteChar(); break;
    case Action::Space: append(' '); break;
    case Action::Shift: shiftActive = !shiftActive; break;
    case Action::Special: specialActive = !specialActive; break;
    case Action::Accept: accept(); return false;
    case Action::Cancel: cancel(); return false;
    default: break;
    }
    // Each text action can only append one byte or delete one byte, so length
    // detects changes without a second buffer/cache. Modes need new key labels.
    if (length_ != previousLength) paintTextField(false);
    else if (key.action == Action::Shift || key.action == Action::Special) drawFrame();
    return false;
}
