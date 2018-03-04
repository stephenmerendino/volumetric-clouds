#pragma once

#include "Engine/Core/StringUtils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class CriticalSection
{
public:
    CRITICAL_SECTION cs;

public:
    CriticalSection();
    ~CriticalSection();

    void lock();
    void unlock();
};

class ScopeCriticalSection
{
public:
    CriticalSection* m_cs;
    
public:
    ScopeCriticalSection(CriticalSection* cs);
    ~ScopeCriticalSection();
};

#define SCOPE_LOCK(csp) ScopeCriticalSection COMBINE(__scs_, __LINE__)(csp)