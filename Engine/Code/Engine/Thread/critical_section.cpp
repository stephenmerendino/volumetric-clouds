#include "Engine/Thread/critical_section.h"

CriticalSection::CriticalSection()
{
    ::InitializeCriticalSection(&cs);
}

CriticalSection::~CriticalSection()
{
    ::DeleteCriticalSection(&cs);
}

void CriticalSection::lock()
{
    ::EnterCriticalSection(&cs);
}

void CriticalSection::unlock()
{
    ::LeaveCriticalSection(&cs);
}

ScopeCriticalSection::ScopeCriticalSection(CriticalSection* cs)
    :m_cs(cs)
{
    m_cs->lock();
}

ScopeCriticalSection::~ScopeCriticalSection()
{
    m_cs->unlock();
}