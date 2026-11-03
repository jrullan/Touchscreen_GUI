#include "Button.h"
#include "IconButton.h"
#include "Keyboard.h"
#include "allocation_hooks.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <new>
template<class Factory>
static HostHeapStats measure(const char* name, Factory factory, std::size_t calls, std::size_t bytes) {
    host_heap_begin();
    HostHeapStats retained;
    {
        auto widget=factory(); // C++17 guaranteed elision: no owned-pointer copies.
        retained=host_heap_stats();
        assert(retained.calls==calls && retained.requested==bytes && retained.retained==bytes);
    }
    assert(host_heap_stats().retained==0);
    host_heap_end();
    std::printf("%s: allocations=%zu requested=%zu retained=%zu after-destroy=0\n",
                name,retained.calls,retained.requested,retained.retained);
    return retained;
}
int main() {
    const auto textBytes=sizeof(Text)+9;
    using Policy=Button::TextStorage;
    std::printf("host sizeof: Button=%zu IconButton=%zu Keyboard=%zu Text=%zu\n",
                sizeof(Button),sizeof(IconButton),sizeof(Keyboard),sizeof(Text));
    measure("Button default",[]{return Button();},2,textBytes);
    measure("Keyboard default",[]{return Keyboard();},2,textBytes);
    measure("Keyboard None",[]{return Keyboard(Policy::None);},0,0);
    measure("Keyboard styled default",[]{return Keyboard(231,197,BLUE,GREEN,RED);},2,textBytes);
    measure("Keyboard styled None",[]{return Keyboard(231,197,BLUE,GREEN,RED,Policy::None);},0,0);
    measure("Icon null-label default",[]{return IconButton(1,1,nullptr,nullptr,nullptr);},2,textBytes);
    measure("Icon null-label None",[]{return IconButton(1,1,nullptr,nullptr,nullptr,Policy::None);},0,0);
    measure("Icon empty-label default",[]{return IconButton(1,1,nullptr);},3,textBytes+1);
    measure("Icon empty-label None",[]{return IconButton(1,1,nullptr,nullptr,(char*)"",Policy::None);},1,1);
    measure("Icon label default",[]{return IconButton(1,1,nullptr,nullptr,(char*)"Lamp");},3,textBytes+5);
    measure("Icon label None",[]{return IconButton(1,1,nullptr,nullptr,(char*)"Lamp",Policy::None);},1,5);
    // Calibrate each C route and ordinary C++ new/delete, not just nothrow Text.
    // --wrap runs at link time, after the compiler's allocation-pair elision.
    // Volatile function pointers make these calibration calls opaque, including
    // operator new/delete: a volatile pointee alone does not prevent new elision.
    decltype(&std::malloc) volatile callMalloc=&std::malloc;
    decltype(&std::calloc) volatile callCalloc=&std::calloc;
    decltype(&std::realloc) volatile callRealloc=&std::realloc;
    decltype(&std::free) volatile callFree=&std::free;
    void* (*volatile callNew)(std::size_t)=&::operator new;
    void (*volatile callDelete)(void*)=&::operator delete;
    host_heap_begin();
    void* p=callMalloc(7); assert(p && host_heap_stats().retained==7);
    p=callRealloc(p,13); assert(p && host_heap_stats().retained==13);
    callFree(p); p=callCalloc(3,5); assert(p && host_heap_stats().retained==15); callFree(p);
    void* integer=callNew(sizeof(int)); assert(host_heap_stats().retained==sizeof(int)); callDelete(integer);
    assert(host_heap_stats().calls==4 && host_heap_stats().requested==7+13+15+sizeof(int));
    assert(host_heap_stats().retained==0); host_heap_end();
}
