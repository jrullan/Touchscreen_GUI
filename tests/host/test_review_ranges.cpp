#include "allocation_hooks.h"
#include "Trend.h"
#include "Gauge.h"
// Hardware stub does not record fillTriangle, so check the private value
// boundary's exact type as well as exercising real Dial rendering below.
#define private public
#include "Dial.h"
#undef private
#include <type_traits>
static_assert(std::is_same<decltype(&Dial::drawNeedle),
    void (Dial::*)(int, int, unsigned int, int, int)>::value,
    "Dial needle must accept currentValue/previousValue without signed narrowing");
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstring>

struct NumberProbe : Indicator {
    int updates = 0;
    void show() override {}
    void update() override { ++updates; }
    const char* text() const { return buf; }
};
struct HostCanvas : Canvas {
    HostCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};

static void unsigned_indicator_values() {
    NumberProbe n;
    n.setLimits(0, UINT_MAX/2, UINT_MAX);
    n.setCV(UINT_MAX, false);
    assert(n.getCV() == UINT_MAX);
    assert(n.updates == 0);
    n.setCV(-1);
    assert(n.getCV() == 0 && n.previousValue == UINT_MAX && n.updates == 1);
    n.setNum(UINT_MAX);
    char expected[64]; std::snprintf(expected, sizeof expected, "%u", UINT_MAX);
    assert(std::strcmp(n.text(), expected) == 0);
    n.setNum(INT_MIN);
    std::snprintf(expected, sizeof expected, "%d", INT_MIN);
    assert(std::strcmp(n.text(), expected) == 0);
    n.setLimits(UINT_MAX-100, UINT_MAX-50, UINT_MAX);
    n.setCV(UINT_MAX-1, false);
    assert(n.getCV() == UINT_MAX-1);
    n.setCV(0, false);
    assert(n.getCV() == UINT_MAX-100);
    n.setLimits(10, 50, 100);
    n.setCV(UINT_MAX, false);
    assert(n.getCV() == 100);
}

template<class Chart> static void unsigned_chart_mapping(Chart& chart) {
    chart.y = 11;
    chart.setLimits(0, UINT_MAX/2, UINT_MAX);
    const int top = chart.y + chart.borderWidth + 1;
    assert(chart.getYVal(UINT_MAX) == top);
    assert(chart.getYVal(0) > top);
    assert(chart.getYVal(-1) == chart.getYVal(0));
    assert(chart.getYVal(UINT_MAX/2) > top);
    chart.setLimits(UINT_MAX-100, UINT_MAX-50, UINT_MAX);
    assert(chart.getYVal(UINT_MAX) == top);
    assert(chart.getYVal(UINT_MAX-100) > chart.getYVal(UINT_MAX-50));
    HostCanvas c; chart.setCanvas(&c); chart.show();
    char expected[64]; std::snprintf(expected, sizeof expected, "text:%u", UINT_MAX);
    bool maxLabel = false;
    for(const auto& op : c.tft->ops) if(op.name == expected) {
        maxLabel = true; assert(op.y == top);
    }
    assert(maxLabel);
    chart.setLimits(7, 7, 7);
    assert(chart.getYVal(UINT_MAX) == chart.getYVal(-1));
    chart.setLimits(UINT_MAX, UINT_MAX, 0);
    assert(chart.getYVal(UINT_MAX) == top);
}

struct DialProbe : Dial {
    DialProbe() : Dial(40, 0, 0, UINT_MAX) {}
    const char* text() const { return buf; }
};
static void unsigned_dial_display() {
    HostCanvas c;
    DialProbe d; d.setCanvas(&c); d.x = d.y = 60;
    d.show();
    c.tft->ops.clear();
    d.setCV(UINT_MAX-1);
    char expected[64]; std::snprintf(expected, sizeof expected, "%u", UINT_MAX-1);
    // Formatting must use Indicator's unsigned buffer, not target-width long.
    assert(std::strcmp(d.text(), expected) == 0);
    bool currentLabel = false;
    for(const auto& op : c.tft->ops)
        if(op.name == std::string("text:")+expected) currentLabel = true;
    assert(currentLabel);
}

static void tiny_y_scale_geometry() {
    HostCanvas c;
    Trend t(40, 1, 0, 50, 100, 2); t.setCanvas(&c);
    t.showXScale = false; t.showYScale = true;
    t.show();
    bool scaleBackground = false;
    for(const auto& op : c.tft->ops) {
        assert(op.w >= 0 && op.h >= 0);
        if(op.name == "fillRect" && op.w != t.w) scaleBackground = true;
    }
    assert(scaleBackground);
    for(int height : {1, 2, 8, 40}) for(bool showX : {false, true}) {
        t.setSize(40, height); t.showXScale = showX;
        c.tft->ops.clear(); t.show();
        for(const auto& op : c.tft->ops) assert(op.w >= 0 && op.h >= 0);
    }
}

static void label_allocation_retry() {
    host_allocation_fail_after = 0;
    NumberProbe n;
    assert(n.text() == nullptr);
    n.clear(); n.setNum(UINT_MAX);
    n.init();
    assert(n.text() != nullptr && n.text()[0] == '\0');
    n.setNum(UINT_MAX);
    char expected[64]; std::snprintf(expected, sizeof expected, "%u", UINT_MAX);
    assert(std::strcmp(n.text(), expected) == 0);
    const char* original = n.text();
    const auto allocations = host_allocation_calls;
    n.init();
    assert(n.text() == original && host_allocation_calls == allocations);
    host_allocation_fail_after = 0;
    Gauge g(40, 100, 0, 50, UINT_MAX);
    HostCanvas c; g.setCanvas(&c); g.show();
    bool maxLabel = false;
    for(const auto& op : c.tft->ops)
        if(op.name == std::string("text:")+expected) maxLabel = true;
    assert(maxLabel); // concrete init retries constructor's failed allocation
}

int main() {
    label_allocation_retry();
    tiny_y_scale_geometry();
    unsigned_dial_display();
    unsigned_indicator_values();
    Gauge g(50, 100, 0, 50, 100); unsigned_chart_mapping(g);
    Trend t(120, 100, 0, 50, 100, 2); unsigned_chart_mapping(t);
    std::puts("review_ranges: PASS");
}
