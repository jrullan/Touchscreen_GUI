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
extern "C" void* __wrap_malloc(std::size_t n){return fail()?nullptr:__real_malloc(n);}
extern "C" void* __wrap_calloc(std::size_t n,std::size_t s){return fail()?nullptr:__real_calloc(n,s);}
extern "C" void* __wrap_realloc(void* p,std::size_t n){return fail()?nullptr:__real_realloc(p,n);}
