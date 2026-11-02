#include "Terminal.h"
#include "allocation_hooks.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>

// The fallback also permits executing this exact test against the base revision:
// it must fail the rotation assertion, rather than fail to compile a missing API.
template<class T>
auto rotating(int w,int h,int dir,int) -> decltype(new T(w,h,dir,1,T::ScrollStorage::RotateRows)) {
    return new T(w,h,dir,1,T::ScrollStorage::RotateRows);
}
template<class T> T* rotating(int w,int h,int dir,long) { return new T(w,h,dir,1); }

#ifdef TERMINAL_SCROLL_MEASURE
static size_t copies=0, copied=0;
extern "C" void* __real_memmove(void*,const void*,size_t);
extern "C" void* __real_memcpy(void*,const void*,size_t);
extern "C" void* __wrap_memmove(void* d,const void* s,size_t n) {
    ++copies; copied+=n; return __real_memmove(d,s,n);
}
extern "C" void* __wrap_memcpy(void* d,const void* s,size_t n) {
    ++copies; copied+=n; return __real_memcpy(d,s,n);
}
#endif

static void same(const Terminal& a,const Terminal& b) {
    assert(a.lines==b.lines && a.maxCharacters==b.maxCharacters && a.linesIndex==b.linesIndex);
    for(int i=0;i<a.lines;++i) {
        assert(!std::memcmp(a.linesBuffer[i],b.linesBuffer[i],a.maxCharacters+1));
        assert(a.linesColors[i]==b.linesColors[i]);
    }
}
static void semantics(int w,int h,int dir) {
    Terminal legacy(w,h,dir);
    std::unique_ptr<Terminal> fast(rotating<Terminal>(w,h,dir,0));
    char* addresses[MAX_LINES];
    for(int i=0;i<legacy.lines;++i) addresses[i]=legacy.linesBuffer[i];
    auto check=[&]() {
        same(legacy,*fast);
        for(int i=0;i<legacy.lines;++i) assert(legacy.linesBuffer[i]==addresses[i]);
        for(int i=0;i<fast->lines;++i) for(int j=0;j<i;++j) assert(fast->linesBuffer[i]!=fast->linesBuffer[j]);
    };
    for(int k=0;k<32;++k) {
        // Embedded newlines, truncation, empty input, partial fill and format semantics.
        const char* text=k%3==0?"A\nB":k%3==1?"abcdefghijklmnopqrstuvwxyz0123456789":"";
        legacy.print(text,RED+k); fast->print(text,RED+k); check();
        if(k%4==0) { legacy.scrollUp(); fast->scrollUp(); check(); }
        if(k%5==0) { legacy.scrollDown(); fast->scrollDown(); check(); }
        if(k==13) { legacy.clear(); fast->clear(); check(); }
        if(k==17) { legacy.direction^=1; fast->direction^=1; }
    }
    legacy.printf((char*)"%d/%d",-123,BLUE); fast->printf((char*)"%d/%d",-123,BLUE); check();
    legacy.print((char*)nullptr); fast->print((char*)nullptr); check();
    // Every byte, including caller-written bytes after NUL, remains compatible.
    for(int i=0;i<legacy.lines;++i) {
        std::memset(legacy.linesBuffer[i],'a'+i,legacy.maxCharacters+1);
        std::memset(fast->linesBuffer[i],'a'+i,fast->maxCharacters+1);
        legacy.linesBuffer[i][0]=fast->linesBuffer[i][0]=0;
    }
    legacy.scrollUp(); fast->scrollUp(); check();
    legacy.scrollDown(); fast->scrollDown(); check();
}
int main(int argc,char**) {
    if(argc==1) {
        std::unique_ptr<Terminal> t(rotating<Terminal>(120,70,TERMINAL_SCROLL_DOWN,0));
        char* old=t->linesBuffer[0];
        t->scrollDown();
        assert(t->linesBuffer[1]==old && "opt-in must rotate storage instead of copying every row");
    }
    for(int dir=0;dir<2;++dir) for(auto dims : {0,1,2,3}) {
        const int widths[]={0,20,74,240}, heights[]={0,26,34,200};
        semantics(widths[dims],heights[dims],dir);
    }
    for(int dir=0;dir<2;++dir) for(int fail=0;fail<=5;++fail) {
        host_allocation_fail_after=fail;
        std::unique_ptr<Terminal> t(rotating<Terminal>(120,70,dir,0));
        assert(t->lines==(fail<5?0:5));
        if(fail<5) for(auto p:t->linesBuffer) assert(!p);
        t->print("safe",RED); t->scrollDown(); t->scrollUp(); t->clear();
        host_allocation_fail_after=-1;
    }
#ifdef TERMINAL_SCROLL_MEASURE
    for(int policy=0;policy<2;++policy) {
        host_heap_begin();
        {
            std::unique_ptr<Terminal> t(policy && argc==1?rotating<Terminal>(120,70,0,0):new Terminal(120,70,0));
            auto heap=host_heap_stats();
            size_t before=host_allocation_calls;
            copies=copied=0;
            for(int i=0;i<100;++i) { t->scrollDown(); t->scrollUp(); }
            std::printf("%s: lines=%u row_bytes=%d scrolls=200 copies=%zu bytes=%zu retained=%zu allocations=%zu sizeof=%zu\n",
                policy && argc==1?"rotating":"legacy",t->lines,t->maxCharacters+1,copies,copied,heap.retained,host_allocation_calls-before,sizeof(Terminal));
            assert(host_allocation_calls==before);
            if(policy && argc==1) assert(copies==200 && copied==200u*(t->maxCharacters+1));
            else assert(copies==200u*(t->lines-1));
            // Append cost includes the same scroll; text insertion itself writes
            // characters directly. No Canvas means renderer/snapshot traffic is
            // deliberately excluded from this storage-only measurement.
            t->direction=TERMINAL_SCROLL_DOWN;
            copies=copied=0;
            for(int i=0;i<200;++i) t->print("append",RED);
            std::printf("%s: appends=200 copies=%zu bytes=%zu constructor_allocations=%zu retained=%zu\n",
                policy && argc==1?"rotating":"legacy",copies,copied,heap.calls,heap.retained);
            assert(copies==200u*(policy && argc==1?1:t->lines-1));
            assert(host_allocation_calls==before);
            // Destroy with a genuinely permuted allocation order.
            t->scrollDown();
        }
        assert(host_heap_stats().retained==0); host_heap_end();
    }
#endif
    std::puts("terminal_scroll: PASS");
}
