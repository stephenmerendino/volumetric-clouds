//-----------------------------------------------------------------------------------------------
// Time.cpp
//	A simple high-precision time utility function for Windows
//	based on code by Squirrel Eiserloh

//-----------------------------------------------------------------------------------------------
#include "Engine/Core/Time.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Engine/Profile/profiler.h"

static uint64_t s_start;

void time_init()
{
    s_start = get_current_perf_counter();
}

double get_current_time_seconds()
{
    uint64_t current = get_current_perf_counter();
    uint64_t elapsed = current - s_start;
    return perf_counter_to_seconds(elapsed);
}