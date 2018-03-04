#pragma once

#include "Engine/Config/build_config.h"
#include "Engine/Core/StringUtils.hpp"

class AutoProfileScope
{
public:
    AutoProfileScope(const char* tag);
    ~AutoProfileScope();
};

#ifdef PROFILED_BUILD
    #define PROFILE_SCOPE(tag)          AutoProfileScope COMBINE(__ps_, __LINE__)(tag);
    #define PROFILE_SCOPE_FUNCTION()    AutoProfileScope COMBINE(__ps_, __LINE__)(__FUNCTION__);
#else
    #define PROFILE_SCOPE(msg)
    #define PROFILE_SCOPE_FUNCTION()
#endif