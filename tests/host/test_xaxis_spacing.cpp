#include "Trend.h"
#include "MultiTrend.h"
#include "allocation_hooks.h"
#include <climits>
#include <cstring>
#include <string>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>

struct AxisCanvas : Canvas {
    AxisCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
template<class Chart> struct AxisProbe : Chart {
    using Chart::Chart;
    using TrendBase::drawXScale;
};
static std::vector<HostDraw> labels(AxisCanvas& c) {
    std::vector<HostDraw> result;
    for(const auto& op : c.tft->ops)
        if(op.name.compare(0,5,"text:")==0) result.push_back(op);
    std::sort(result.begin(),result.end(),[](const HostDraw& a,const HostDraw& b){return a.x<b.x;});
    return result;
}
static void expectSpacing(AxisCanvas& c, TrendBase& chart) {
    auto text=labels(c);
    for(size_t i=0;i<text.size();++i) {
        const auto& op=text[i];
        if(i && text[i-1].x+text[i-1].w+2>op.x) {
            std::fprintf(stderr,"overlap: %s [%d,%d) vs %s [%d,%d)\n",
                text[i-1].name.c_str(),text[i-1].x,text[i-1].x+text[i-1].w,
                op.name.c_str(),op.x,op.x+op.w);
            assert(false && "X labels require a two-pixel gap");
        }
    }
    for(const auto& op:text) {
        assert(op.x>=chart.x && op.x+op.w<=chart.x+chart.w);
        assert(op.y>=chart.y && op.y+op.h<=chart.y+chart.h);
    }
}
static std::string expectedText(int first,int offset,int span,int maxX) {
    const int value=maxX>0 ? (span ? int(int64_t(offset)*maxX/span) : 0) : first+offset;
    char out[64];
    if(maxX>0 && value>=60) std::snprintf(out,sizeof out,"%d:%02d",value/60,value%60);
    else std::snprintf(out,sizeof out,"%d",value);
    return std::string("text:")+out;
}
static bool contains(const std::vector<HostDraw>& text,const std::string& value) {
    for(const auto& op:text) if(op.name==value) return true;
    return false;
}
template<class Chart> static void matrix(AxisProbe<Chart>& t,AxisCanvas& c) {
    t.setCanvas(&c); t.x=7; t.y=9;
    unsigned cases=0;
    for(int width : {0,1,5,6,12,18,34,35,40,60,120,230})
    for(int font : {0,1,2,31,255})
    for(bool ys : {false,true})
    for(int seconds : {0,-1,59,60,61,599,600,601,3599,3600,INT_MAX})
    for(auto window : {std::pair<int,int>{0,0},{0,1},{2,5},{9,11},{0,15},{0,31},{90,110},{0,254},{253,254}}) {
        t.w=width; t.h=100; t.fontSize=font; t.showYScale=ys;
        t.setWindow(window.first,window.second); t.setMaxX(seconds);
        const int first=t.trendWindow.minValue,span=t.trendWindow.maxValue-first;
        const int mappedFirst=t.getXVal(first),mappedLast=t.getXVal(first+span);
        c.tft->ops.clear();
        const auto allocations=host_allocation_calls;
        t.drawXScale();
        assert(host_allocation_calls==allocations);
        expectSpacing(c,t);
        auto text=labels(c);
        for(const auto& op:c.tft->ops) {
            assert(op.x>=t.x && op.y>=t.y && op.w>=0 && op.h>=0);
            assert(op.x+op.w<=t.x+t.w && op.y+op.h<=t.y+t.h);
        }
        for(const auto& op:text) {
            assert(op.w==int(op.name.size()-5)*6 && op.h==8);
            bool candidate=false;
            for(int off=0;off<=span;++off)
                if((off%2==0 || off==span) && op.name==expectedText(first,off,span,seconds)) candidate=true;
            assert(candidate);
        }
        if(width>(ys?34:0)) {
            const auto right=expectedText(first,0,span,seconds),left=expectedText(first,span,span,seconds);
            const int rw=int(right.size()-5)*6,lw=int(left.size()-5)*6;
            if(rw<=width) assert(contains(text,right));
            if(span && rw<=width && lw<=width) {
                const int rx=std::max(t.x,std::min(mappedFirst-rw/2,t.x+width-rw));
                const int lx=std::max(t.x,std::min(mappedLast-lw/2,t.x+width-lw));
                if(lx+lw+2<=rx) assert(contains(text,left));
            }
        } else assert(text.empty());
        assert(t.getXVal(first)==mappedFirst && t.getXVal(first+span)==mappedLast);
        ++cases;
    }
    std::printf("matrix cases=%u\n",cases);
}
template<class Chart> static void compatibility(AxisProbe<Chart>& t,AxisCanvas& c) {
    t.setCanvas(&c); t.x=7; t.y=9; t.w=230; t.h=100;
    t.showYScale=false; t.showThresholds=false; t.setWindow(2,5); t.setMaxX(0);
    for(int k=0;k<t.maxValues;++k) t.values[k]=uint8_t(k%101);
    std::vector<uint8_t> saved(t.values,t.values+t.maxValues);
    c.tft->ops.clear(); t.drawXScale();
    auto text=labels(c);
    assert(text.size()==3 && contains(text,"text:2") && contains(text,"text:4") && contains(text,"text:5"));
    assert(t.getXVal(2)==t.x+227 && t.getXVal(5)==t.x+2);
    assert(std::equal(saved.begin(),saved.end(),t.values));
    t.setMaxX(INT_MAX); c.tft->ops.clear(); t.drawXScale();
    assert(contains(labels(c),"text:35791394:07"));
    t.setMaxX(0); t.setWindow(254,254); t.w=12;
    c.tft->ops.clear(); t.drawXScale(); assert(labels(c).empty()); // 18px cannot fit
    t.w=6; t.setWindow(0,254); c.tft->ops.clear(); t.drawXScale();
    assert(labels(c).size()==1); // never force a colliding second endpoint
    for(int height : {0,1,8,20,26,27,28,100}) {
        t.h=height; c.tft->ops.clear(); t.drawXScale(); expectSpacing(c,t);
        for(const auto& op:c.tft->ops)
            assert(op.y>=t.y && op.y+op.h<=t.y+t.h);
    }
    t.h=100; t.w=120; t.showXScale=false;
    c.tft->ops.clear(); t.drawXScale(); assert(c.tft->ops.empty());
    t.showXScale=true; t.show();
    c.tft->ops.clear();
    const auto allocations=host_allocation_calls;
    for(int k=0;k<10;++k) t.update();
    assert(c.tft->ops.empty() && host_allocation_calls==allocations);
    t.showXScale=false; c.tft->ops.clear(); t.update();
    assert(labels(c).empty());
    c.tft->ops.clear(); t.update(); assert(c.tft->ops.empty());
}
int main() {
    AxisCanvas c;
    AxisProbe<Trend> t(120,100,0,50,100,255);
    t.setCanvas(&c); t.x=7; t.y=9;
    t.drawXScale(); expectSpacing(c,t);
    matrix(t,c); compatibility(t,c);
    AxisProbe<MultiTrend> m(120,100,0,50,100,2,255);
    matrix(m,c); compatibility(m,c);
    std::puts("xaxis_spacing: PASS");
}
