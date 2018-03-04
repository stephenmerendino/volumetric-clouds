#include "Engine/Profile/auto_profile_scope.h"
#include "Engine/Profile/profiler.h"

AutoProfileScope::AutoProfileScope(const char* tag)
{
    profiler_push(tag);
}

AutoProfileScope::~AutoProfileScope()
{
    profiler_pop();
}