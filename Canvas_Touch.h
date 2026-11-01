#ifndef CANVAS_TOUCH_H
#define CANVAS_TOUCH_H

#include "Canvas.h"
#include "TouchMapper.h"

// Opt-in canvas: mapper and its driver are BORROWED and must outlive this
// canvas. Base Canvas owns tft and the published Touch bridge, never mapper.
class Canvas_Touch : public Canvas {
public:
    Canvas_Touch(int mode, int color, TouchMapper* mapper,
                 int tft_cs = -1, int tft_dc = -1)
        : Canvas(mode, color, tft_cs, tft_dc), mapper_(mapper),
          pending_(nullptr), displayReady_(false), tftCS_(tft_cs), tftDC_(tft_dc),
          rotation_(mode == TFT_LANDSCAPE ? 3 : 0) {
        if (tft) init();
    }
    ~Canvas_Touch() override { delete pending_; }
    bool isReady() const { return initialized; }
    void init() override {
        if (initialized || !mapper_) return;
        if (!tft) tft = new (std::nothrow) Guitft(tftCS_, tftDC_);
        if (!tft) return;
        if (!displayReady_) {
            tft->begin();
            tft->fillScreen(bgColor);
            setRotation(rotation_);
            displayReady_ = true;
        }
        if (!pending_) pending_ = new (std::nothrow) Bridge(mapper_);
        if (!pending_) return;
        deselectDisplay();
        if (!mapper_->begin()) return;
        lastMillis = touchSampling = scanSampling = millis();
        scanSampleDelay = SCAN_SAMPLING_TIME;
        ts = pending_;
        pending_ = nullptr;
        initialized = true;
    }
    void setRotation(uint8_t rotation) {
        rotation_ = rotation % 4;
        _mode = (rotation_ & 1) ? TFT_LANDSCAPE : TFT_PORTRAIT;
        w = (rotation_ & 1) ? 320 : 240;
        h = (rotation_ & 1) ? 240 : 320;
        if (mapper_) mapper_->setOrientation(rotation_);
        if (tft) {
            tft->layoutMode = _mode;
            tft->setRotation(rotation_);
        }
    }
    void portrait() override { setRotation(0); }
    void landscape() override { setRotation(3); }
    Point* getTouchedPoint() override {
        if (!initialized || !tft || !ts || !mapper_->isReady()) return nullptr;
        const unsigned long now = millis();
        if (static_cast<unsigned long>(now - touchSampling) <= TOUCH_SAMPLING_TIME)
            return nullptr;
        touchSampling = now;
        deselectDisplay();
        mapper_->poll();
        if (mapper_->state() != TOUCH_PRESSED && mapper_->state() != TOUCH_HELD)
            return nullptr;
        touchedPoint = mapper_->currentPoint();
        return &touchedPoint;
    }
private:
    // This private bridge's touched() means "ready to sample", not "pressed".
    // Canvas::scan still owns its sampling and propagation order; only
    // getTouchedPoint will poll the mapper and decide whether to dispatch.
    class Bridge : public Touch {
    public:
        explicit Bridge(TouchMapper* mapper) : mapper_(mapper) {}
        bool begin(uint8_t) override { return mapper_->isReady(); }
        uint8_t touched() override { return mapper_->isReady(); }
        Point getPoint() override { return mapper_->currentPoint(); }
    private:
        TouchMapper* mapper_;
    };
    void deselectDisplay() {
        if (tftCS_ >= 0) { pinMode(tftCS_, OUTPUT); digitalWrite(tftCS_, HIGH); }
    }
    TouchMapper* mapper_;
    Bridge* pending_;
    bool displayReady_;
    int tftCS_, tftDC_;
    uint8_t rotation_;
};
#endif
