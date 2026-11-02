#include "allocation_hooks.h"
#include <cstdlib>
#include <new>
long host_nothrow_new_fail_after=-1;
void* operator new(std::size_t n, const std::nothrow_t&) noexcept {
 if(host_nothrow_new_fail_after==0){host_nothrow_new_fail_after=-1;return nullptr;}
 if(host_nothrow_new_fail_after>0)--host_nothrow_new_fail_after;
 try{return ::operator new(n);}catch(...){return nullptr;}
}
long host_allocation_fail_after=-1;
std::size_t host_allocation_calls=0;
extern "C" void* __real_malloc(std::size_t);
extern "C" void* __real_calloc(std::size_t,std::size_t);
extern "C" void* __real_realloc(void*,std::size_t);
static bool fail(){++host_allocation_calls;if(host_allocation_fail_after<0)return false;if(host_allocation_fail_after==0){host_allocation_fail_after=-1;return true;}--host_allocation_fail_after;return false;}
#ifdef HOST_HEAP_MEASURE
#include <cassert>
namespace {
struct Record { void* p; std::size_t n; };
Record records[512] = {};
HostHeapStats stats = {};
bool measuring = false;
void remember(void* p, std::size_t n) {
 if (!measuring || !p) return;
 ++stats.calls; stats.requested += n; stats.retained += n;
 for (auto& r : records) if (!r.p) { r={p,n}; return; }
 std::abort(); // Never silently undercount.
}
void forget(void* p) {
 if (!p) return;
 for (auto& r : records) if (r.p==p) { stats.retained-=r.n; r={}; return; }
}
}
void host_heap_begin() { assert(!measuring && stats.retained==0); stats={}; measuring=true; }
HostHeapStats host_heap_stats() { return stats; }
void host_heap_end() { assert(stats.retained==0); measuring=false; }
extern "C" void __real_free(void*);
extern "C" void __wrap_free(void* p) { forget(p); __real_free(p); }
// Delegate to real C++ allocation so ASan's new/delete pairing stays intact.
extern "C" void* __real__Znwm(std::size_t);
extern "C" void* __wrap__Znwm(std::size_t n) { void* p=__real__Znwm(n); remember(p,n); return p; }
extern "C" void __real__ZdlPv(void*);
extern "C" void __wrap__ZdlPv(void* p) { forget(p); __real__ZdlPv(p); }
extern "C" void __real__ZdlPvm(void*,std::size_t);
extern "C" void __wrap__ZdlPvm(void* p,std::size_t n) { forget(p); __real__ZdlPvm(p,n); }
#else
static void remember(void*,std::size_t) {}
static void forget(void*) {}
#endif
extern "C" void* __wrap_malloc(std::size_t n){void* p=fail()?nullptr:__real_malloc(n);remember(p,n);return p;}
extern "C" void* __wrap_calloc(std::size_t n,std::size_t s){void* p=fail()?nullptr:__real_calloc(n,s);remember(p,n*s);return p;}
extern "C" void* __wrap_realloc(void* p,std::size_t n){if(fail())return nullptr;void* q=__real_realloc(p,n);if(q || n==0){forget(p);remember(q,n);}return q;}
