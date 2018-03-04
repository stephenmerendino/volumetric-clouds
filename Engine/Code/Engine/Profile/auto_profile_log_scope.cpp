#include "Engine/Profile/auto_profile_log_scope.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Profile/profiler.h"
#include "Engine/Core/log.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

AutoProfileLogScope::AutoProfileLogScope(const char* msg)
    :m_message(msg)
{
    m_start = get_current_perf_counter();
}

AutoProfileLogScope::~AutoProfileLogScope()
{
    uint64_t end = get_current_perf_counter();
    uint64_t delta = end - m_start;

    double seconds_elapsed = perf_counter_to_seconds(delta);
    double ms_elapsed = seconds_elapsed * 1000.0f;

    log_tagged_printf("profiler", "'%s' finished. Elapsed time: %f ms, Ops: %ld", m_message.c_str(), ms_elapsed, delta);
}