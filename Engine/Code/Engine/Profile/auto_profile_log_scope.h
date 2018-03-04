#pragma once

#include "Engine/Config/build_config.h"
#include "Engine/Core/StringUtils.hpp"

#include <string>
#include <cstdint>

class AutoProfileLogScope
{
public:
    std::uint64_t   m_start; 
    std::string     m_message;

public:
    AutoProfileLogScope(const char* msg);
    ~AutoProfileLogScope();
};

#define PROFILE_LOG_SCOPE(msg)          AutoProfileLogScope COMBINE(__pls_, __LINE__)(msg);
#define PROFILE_LOG_SCOPE_FUNCTION()    AutoProfileLogScope COMBINE(__pls_, __LINE__)(__FUNCTION__);