// Characterization/regression of the existing opt-in capacity, not a new feature.
#include "MultiTrend.h"
#include "allocation_hooks.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>

struct CapacityCanvas : Canvas {
    CapacityCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};

static HostHeapStats measure(int s, int n, bool exact, CapacityCanvas& canvas) {
    // Exclude the hardware seam's pixel/operation buffers from widget accounting.
    canvas.tft->ops.clear();
    host_heap_begin();
    HostHeapStats ctor, render, update;
    {
        auto m = exact ? MultiTrend(180,140,0,50,100,s,n,s)
                       : MultiTrend(180,140,0,50,100,s,n);
        ctor=host_heap_stats();
        const size_t capacity=exact?s:MAX_MULTITREND_SERIES;
        assert(ctor.calls==3);
        assert(ctor.requested==BUF_SIZE+1+capacity*(n+sizeof(int)));
        assert(ctor.retained==ctor.requested);
        m.setCanvas(&canvas); m.x=10; m.y=10;
        for(int k=0;k<n;++k) for(int j=0;j<s;++j) m.addValue(j,(k+j*23)%101,false);
        m.setMarkerRadius(2); m.show();
        render=host_heap_stats();
        const size_t cache=(n*(s+1)+3*s)*sizeof(uint16_t);
        assert(render.calls==ctor.calls+1);
        assert(render.retained==ctor.retained+cache);
        for(int j=0;j<s;++j) m.addValue(j,71+j,false);
        m.update(); update=host_heap_stats();
        assert(update.calls==render.calls && update.retained==render.retained);
    }
    const auto destroyed=host_heap_stats();
    assert(destroyed.retained==0); host_heap_end();
    std::printf("S=%d N=%d %s ctor_calls=%zu requested=%zu retained=%zu first_render=%zu update=%zu after_destroy=%zu\n",
        s,n,exact?"exact":"default4",ctor.calls,ctor.requested,ctor.retained,
        render.retained,update.retained,destroyed.retained);
    return ctor;
}

static void behavior(int s, int n) {
    MultiTrend m(180,140,0,50,100,s,n,s);
    assert(m.maxSeries==s && m.numSeries==s && m.maxValues==n);
    for(int k=0;k<n+9;++k) for(int j=0;j<s;++j) m.addValue(j,(k+j*23)%201,false);
    for(int j=0;j<s;++j) {
        int lo=255,hi=0;
        for(int i=0;i<n;++i) {
            const int expected=(n+8-i+j*23)%201;
            assert(m.values[j*n+i]==expected);
            lo=std::min(lo,expected); hi=std::max(hi,expected);
        }
        assert(m.latest(j)==m.values[j*n]);
        assert(m.getMin(j)==lo && m.getMax(j)==hi);
    }
    int lo=255,hi=0;
    for(int i=0;i<s*n;++i) { lo=std::min(lo,int(m.values[i])); hi=std::max(hi,int(m.values[i])); }
    m.setWindow(2,5); // min/max/autofit consider full history, not just the window.
    m.autoFit(true);
    assert(m.scaleMin==unsigned(lo>2?lo-2:0) && m.scaleMax==unsigned(hi+2));
    const auto min=m.scaleMin,max=m.scaleMax;
    m.autoFit(false); assert(m.scaleMin==min && m.scaleMax==max);
    std::vector<uint8_t> history(m.values,m.values+s*n);
    std::vector<int> colors(m.seriesColor,m.seriesColor+s);
    const auto current=m.currentValue;
    for(int j=s;j<=255;++j) {
        m.addValue(j,255,false); m.setSeriesColor(j,0x1234);
        assert(m.latest(j)==0 && m.getMin(j)==int(min) && m.getMax(j)==int(max));
    }
    assert(std::equal(history.begin(),history.end(),m.values));
    assert(std::equal(colors.begin(),colors.end(),m.seriesColor) && m.currentValue==current);
    m.clear();
    assert(std::all_of(m.values,m.values+s*n,[](uint8_t v){return v==0;}));
    assert(std::equal(colors.begin(),colors.end(),m.seriesColor));
    m.addValue(s-1,99,false); m.init();
    assert(m.latest(s-1)==0 && m.maxSeries==s && m.numSeries==s);
    assert(m.trendWindow.minValue==0 && m.trendWindow.maxValue==n-1);
}

static void oom(int s,int n,CapacityCanvas& canvas) {
    for(int failure : {0,1}) {
        // Constructor has a separate base numeric-label malloc before these two.
        host_allocation_fail_after=1+failure;
        {
            MultiTrend constructed(180,140,0,50,100,s,n,s);
            assert(host_allocation_fail_after==-1);
            assert((constructed.values==nullptr)==(failure==0));
            assert((constructed.seriesColor==nullptr)==(failure==1));
            constructed.setCanvas(&canvas); constructed.setMarkerRadius(2);
            constructed.addValue(s-1,61); constructed.show(); constructed.autoFit(true);
            constructed.init();
            assert(constructed.values && constructed.seriesColor);
        }
        // Reinit isolates precisely history malloc (0) and palette malloc (1),
        // avoiding Indicator's separate numeric-label allocation.
        MultiTrend m(180,140,0,50,100,s,n,s);
        m.setCanvas(&canvas); m.show();
        host_allocation_fail_after=failure; m.init();
        assert(host_allocation_fail_after==-1);
        assert((m.values==nullptr)==(failure==0));
        assert((m.seriesColor==nullptr)==(failure==1));
        m.setMarkerRadius(2); m.clear(); m.addValue(s-1,73);
        m.autoFit(true); m.drawTrend(); m.eraseTrend(); m.show(); m.forceUpdate();
        assert(m.latest(s-1)==(failure==0?0:73));
        m.init(); assert(m.values && m.seriesColor && m.latest(s-1)==0);
        m.addValue(s-1,91,false); assert(m.latest(s-1)==91);
    }
}

int main() {
    CapacityCanvas canvas;
    canvas.tft->ops.reserve(200000); // short recorded labels use string SSO;
    // Per-phase assertions fail if instrumentation or the seam adds allocations.
    std::printf("host sizeof(int)=%zu sizeof(MultiTrend)=%zu numeric_label=%zu; requested bytes exclude allocator overhead\n",sizeof(int),sizeof(MultiTrend),BUF_SIZE+1);
    for(int n : {16,32,255}) for(int s : {1,2,3,4}) {
        auto baseline=measure(s,n,false,canvas), exact=measure(s,n,true,canvas);
        std::printf("S=%d N=%d measured_constructor_retained_delta=%zu\n",s,n,baseline.retained-exact.retained);
        behavior(s,n); oom(s,n,canvas);
    }
    std::puts("multitrend capacity characterization/regression: PASS");
}
