#pragma once
#include <cstddef>
// Set to zero to fail the next wrapped C allocation; -1 disables injection.
extern long host_allocation_fail_after;
extern long host_nothrow_new_fail_after;
extern std::size_t host_allocation_calls;
