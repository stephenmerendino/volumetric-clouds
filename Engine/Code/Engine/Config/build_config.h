#pragma once

#include "Engine/Core/StringUtils.hpp"

// -----------------------------------------
// Memory Tracking
#define MEMORY_TRACKER_FRAME_HISTORY    720

#define TRACK_MEMORY_BASIC              (0)
#define TRACK_MEMORY_VERBOSE            (1)

#if defined(FINAL_BUILD)
    // no memory tracking
    // no profiling
#elif defined(_DEBUG)
    #define TRACK_MEMORY TRACK_MEMORY_VERBOSE
    #define PROFILED_BUILD
#else 
    // release build
    #define TRACK_MEMORY TRACK_MEMORY_BASIC
#endif

// -----------------------------------------
// Profiling
#define PROFILER_FRAME_HISTORY          256

// -----------------------------------------
// Logging
#define LOG_FILE_HISTORY                3
#define LOG_FILE_DIRECTORY              "Log/"
#define LOG_DATE_FORMAT                 "%Y%m%d_%H%M%S"
#define LOG_TIMESTAMP_FORMAT            "log_%s_%i.txt"
#define LOG_PURGE_SEARCH_STRING         "log_*.txt"