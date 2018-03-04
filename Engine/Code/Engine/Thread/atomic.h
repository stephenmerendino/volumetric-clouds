#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>

__forceinline
unsigned int atomic_add(unsigned int volatile *ptr, unsigned int const value)
{
    return (unsigned int) ::InterlockedAddNoFence((LONG volatile*)ptr, (LONG)value);
}

__forceinline
unsigned int atomic_incr(unsigned int *ptr)
{
    return (unsigned int) ::InterlockedIncrementNoFence((LONG volatile*)ptr);
}

__forceinline
unsigned int atomic_decr(unsigned int *ptr)
{
    return (unsigned int) ::InterlockedDecrementNoFence((LONG volatile*)ptr);
}

__forceinline
unsigned int compare_and_set(unsigned int volatile *ptr, unsigned int const comparand, unsigned int const value)
{
    return ::InterlockedCompareExchange(ptr, value, comparand);
}

template <typename T>
__forceinline T* compare_and_set_ptr(T *volatile *ptr, T *comparand, T *value)
{
    return (T*)::InterlockedCompareExchangePointerNoFence((PVOID volatile*)ptr, (PVOID)value, (PVOID)comparand);
}