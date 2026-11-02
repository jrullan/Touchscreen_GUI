#pragma once
#include <cstddef>
// Set to zero to fail the next wrapped C allocation; -1 disables injection.
extern long host_allocation_fail_after;
extern long host_nothrow_new_fail_after;
extern std::size_t host_allocation_calls;
// Optional linker-wrapped measurement: requested bytes, not allocator overhead.
struct HostHeapStats { std::size_t calls, requested, retained; };
void host_heap_begin();
HostHeapStats host_heap_stats();
void host_heap_end();
