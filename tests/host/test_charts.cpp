#include "allocation_hooks.h"
#include "Trend.h"
#include "MultiTrend.h"
#include "Gauge.h"
#include "Dial.h"
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstring>
#include <unistd.h>

struct NumberProbe : Indicator {
    void show() override {}
    void update() override {}
    const char* text() const { return buf; }
};

struct HostCanvas : Canvas {
    HostCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};

int main() {
    NumberProbe n;
    n.setNum(-123);
    assert(std::strcmp(n.text(), "-123") == 0);
    for (int value : {INT_MIN, INT_MAX, 0, -1}) {
        char expected[64]; std::snprintf(expected, sizeof expected, "%d", value);
        n.setNum(value); assert(std::strcmp(n.text(), expected) == 0);
    }
    { Gauge g(20, 100, 0, 50, 100); Dial d(40, 0, 50, 100); }
    {
        Trend empty(100, 100, 0, 50, 100, 0);
        empty.visible = false;
        assert(empty.getMin() == 0);
        assert(empty.getMax() == 100);
        empty.addValue(7, false);
    }
    {
        Trend t(120, 100, 0, 50, 100, 8);
        t.showYScale = t.showXScale = false;
        t.setWindow(2, 5);
        assert(t.getXVal(2) == 117);
        assert(t.getXVal(5) == 2);
        t.setWindow(-100, 100);
        assert(t.trendWindow.minValue == 0 && t.trendWindow.maxValue == 7);
        t.setWindow(6, 2);
        assert(t.trendWindow.minValue <= t.trendWindow.maxValue);
        t.setWindow(3, 3);
        assert(t.getXVal(3) >= 0 && t.getXVal(3) < t.w);
    }
    {
        Trend t(120, 100, 10, 10, 10, 1);
        t.showYScale = t.showXScale = false;
        assert(t.getYVal(10) >= 0 && t.getYVal(10) < t.h);
        Gauge g(20, 100, 10, 10, 10);
        assert(g.getYVal(10) >= 0 && g.getYVal(10) < g.h);
        t.setLimits(100, 500, 10);
        assert(t.scaleMin == 10 && t.scaleMax == 100 && t.setpoint == 100);
        t.setCV(-1, false);
        assert(t.getCV() == 10);
    }
    {
        MultiTrend m(120, 100, 0, 50, 100, 255, 4, 255);
        assert(m.maxSeries == MAX_MULTITREND_SERIES);
        assert(m.numSeries == MAX_MULTITREND_SERIES);
        m.visible = false;
        m.addValue(0, 11, false); m.addValue(0, 22, false);
        assert(m.values[0] == 22 && m.values[1] == 11);
        m.init(); assert(m.latest(0) == 0);
        assert(m.getMin(255) == 0 && m.getMax(255) == 100);
        MultiTrend empty(10, 10, 0, 5, 10, 0, 0, 0);
        assert(empty.latest(0) == 0);
        empty.addValue(0, 3, false);
    }
    {
        Trend t(120, 100, 0, 50, 100, 4);
        t.addValue(5); t.show(); t.update(); t.drawTrend(); t.eraseTrend();
        t.enableAutoFit = true; t.forceUpdate(); t.autoFit(true);
        MultiTrend m(120, 100, 0, 50, 100);
        m.addValue(0, 5); m.show(); m.forceUpdate(); m.drawTrend(); m.eraseTrend();
        m.enableAutoFit = true; m.autoFit(true);
        Gauge g(20, 100, 0, 50, 100);
        g.show(); g.update(); g.drawFill(); g.drawBorder();
        Dial d(40, 0, 50, 100); d.show(); d.setCV(4); d.forceUpdate();
    }
    {
        HostCanvas c;
        Trend t(120, 100, 0, 50, 100, 3);
        t.setCanvas(&c); t.showYScale = t.showXScale = t.showThresholds = false;
        t.forceSquareWaveform = true;
        t.values[0] = 0; t.values[1] = 100; t.values[2] = 0;
        t.drawTrend();
        unsigned verticals = 0;
        for(const auto& op : c.tft->ops) if(op.name == "vline") {
            ++verticals; assert(op.h > 0 && op.h <= t.h);
            assert(op.y == t.getYVal(100));
        }
        assert(verticals == 2);
    }
    {
        HostCanvas c;
        Trend t(40, 100, 0, 50, 100, 4);
        t.setCanvas(&c); t.show();
        for(const auto& op : c.tft->ops) {
            assert(op.w >= 0 && op.h >= 0);
        }
    }
    {
        host_allocation_fail_after = 0;
        NumberProbe failed;
        assert(failed.text() == nullptr);
        failed.clear(); failed.setNum(INT_MIN);
    }
    {
        HostCanvas c;
        host_allocation_fail_after = 1; // base label succeeds, history fails
        Trend t(120, 100, 0, 50, 100, 4);
        assert(t.values == nullptr); t.setCanvas(&c); t.enableAutoFit = true;
        t.addValue(8); t.show(); t.update(); t.forceUpdate(); t.autoFit(true);
        t.init(); assert(t.values != nullptr); t.addValue(8, false);
        host_allocation_fail_after = 0;
        t.init(); assert(t.values == nullptr); t.update();
        host_allocation_fail_after = 1;
        MultiTrend m(120, 100, 0, 50, 100);
        assert(m.values == nullptr); m.setCanvas(&c); m.enableAutoFit = true;
        m.setMarkerRadius(2); m.addValue(0, 3); m.show(); m.forceUpdate();
        m.autoFit(true); assert(m.getMin(0) == (int)m.scaleMin);
        m.init(); assert(m.values); m.addValue(0, 42, false);
        host_allocation_fail_after = 1; // history succeeds, palette fails
        m.init(); assert(m.values && !m.seriesColor); m.update();
    }
    {
        Trend t(100, 100, 10, 50, 100, 1);
        t.addValue(1, false); t.autoFit(true);
        assert(t.scaleMin == 0 && t.scaleMax == 3);
        MultiTrend m(100, 100, 10, 50, 100, 1, 1);
        m.addValue(0, 1, false); m.autoFit(true);
        assert(m.scaleMin == 0 && m.scaleMax == 3);
        m.numSeries = 0; m.setLimits(10, 50, 100); m.autoFit(true);
        assert(m.scaleMin == 10 && m.scaleMax == 100);
    }
    {
        HostCanvas c;
        Gauge g(20, 400, 0, 50, 100); g.setCanvas(&c); g.y = 300;
        g.drawBorder();
        assert(!c.tft->ops.empty() && c.tft->ops[0].y == 300);
        assert(c.tft->ops[0].h == 400);
    }
    {
        HostCanvas c;
        Gauge g(1, 1, 10, 10, 10); g.setCanvas(&c); g.show();
        Dial d(2, 10, 10, 10); d.setCanvas(&c); d.show(); d.forceUpdate();
        Trend t(1, 1, 10, 10, 10, 1); t.setCanvas(&c); t.show();
        for(const auto& op : c.tft->ops) assert(op.w >= 0 && op.h >= 0);
    }
    {
        Trend t;
        assert(t.maxValues == MAX_TREND_VALUES && t.values);
        t.init(); t.addValue(7, false); assert(t.values[0] == 7);
        t.init(); assert(t.getMin() == 0);
        Gauge g; Dial d;
        assert(g.scaleMin == 0 && g.scaleMax == 100);
        assert(g.getCV() == 0 && d.getCV() == 0 && d.radius == 0);
        g.clear(); d.clear();
    }
    {
        HostCanvas c;
        Trend t(120, 100, 0, 50, 100, 8);
        t.setCanvas(&c); t.setWindow(2, 5); t.setMaxX(1200); t.show();
        bool duration = false, oldestTick = false;
        for(const auto& op : c.tft->ops) {
            if(op.name == "text:20:00") duration = true;
            if(op.name == "vline" && op.x == t.getXVal(5)) oldestTick = true;
        }
        assert(duration && oldestTick);
    }
    {
        HostCanvas c;
        MultiTrend m(120, 100, 0, 50, 100, 1, 4, 1);
        m.setCanvas(&c); m.setMarkerRadius(2); m.numSeries = 255;
        m.drawTrend(); m.eraseTrend(); m.enableAutoFit = true; m.update();
    }
    {
        Trend t(1, 1, 0, 50, 100, 1);
        assert(t.getXVal(0) == t.x);
        t.rightInset = 255; t.topInset = t.bottomInset = 255;
        assert(t.getXVal(0) == t.x && t.getYVal(50) == t.y);
    }
    {
        Gauge g(20, 100, 10, 30, 50);
        assert(g.hiLimit == 50 && g.lowLimit == 10 && g.currentValue == 30);
    }
    {
        HostCanvas c;
        for(int failedAllocation = 0; failedAllocation < 3; ++failedAllocation) {
            host_allocation_fail_after = failedAllocation;
            MultiTrend m(120, 100, 0, 50, 100, 2, 1);
            m.setCanvas(&c); m.enableAutoFit = true; m.setMarkerRadius(2);
            m.clear(); m.addValue(0, 42); m.drawTrend(); m.eraseTrend(); m.show();
            m.forceUpdate(); m.autoFit(true); m.init();
        }
        host_allocation_fail_after = 0;
        Gauge g(20, 100, 0, 50, 100); g.setCanvas(&c);
        g.clear(); g.setNum(INT_MAX); g.show(); g.update();
        host_allocation_fail_after = 0;
        Dial d(40, 0, 50, 100); d.setCanvas(&c);
        d.clear(); d.setNum(INT_MIN); d.show(); d.forceUpdate();
        Trend t(120, 100, 0, 50, 100, 1); t.setCanvas(&c);
        t.enableAutoFit = true; t.addValue(42); t.forceUpdate();
        assert(t.values[0] == 42);
        MultiTrend m(120, 100, 0, 50, 100, 1, 1); m.setCanvas(&c);
        m.setMarkerRadius(2); m.addValue(0, 42); m.forceUpdate();
        assert(m.latest(0) == 42);
        c.tft->ops.clear();
        t.visible = m.visible = g.visible = d.visible = false;
        // Mutation/update honors hidden state; explicit show() is tested
        // separately in test_visibility.cpp and intentionally makes it visible.
        t.addValue(17); t.forceUpdate(); t.drawTrend();
        m.addValue(0, 17); m.forceUpdate(); m.drawTrend();
        g.update(); g.drawBorder(); g.drawFill(); d.forceUpdate();
        assert(c.tft->ops.empty() && t.values[0] == 17 && m.latest(0) == 17);
        t.visible = m.visible = g.visible = d.visible = true;
        Guitft* display = c.tft; c.tft = nullptr;
        t.show(); t.forceUpdate(); m.show(); m.forceUpdate(); g.show(); d.show();
        c.tft = display;
        host_allocation_fail_after = -1;
    }
    {
        HostCanvas c;
        Dial d(40, 0, 50, 100); d.setCanvas(&c); d.tickDegree = 0;
        alarm(2); d.show(); alarm(0); // a zero tick step must not hang
    }
    {
        Trend t(120, 100, 0, 50, 100, 8);
        t.showYScale = false;
        t.trendWindow.minValue = INT_MIN; t.trendWindow.maxValue = INT_MAX;
        assert(t.getXVal(0) == 117 && t.getXVal(7) == 2);
    }
    std::puts("charts: PASS");
}
