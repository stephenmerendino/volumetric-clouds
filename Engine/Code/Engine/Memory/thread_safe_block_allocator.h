#pragma once

#include "Engine/Memory/base_allocator.h"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Thread/thread.h"
#include <cstdlib>

class ThreadSafeBlockAllocator : public BaseAllocator
{
private:
    struct block_t
    {
        block_t *next;
    };

public:
    size_t block_size;
    block_t *free_list;
    unsigned int alloc_count;

    CriticalSection lock;

public:
    ThreadSafeBlockAllocator::ThreadSafeBlockAllocator(size_t bs);
    ThreadSafeBlockAllocator::~ThreadSafeBlockAllocator();

    void* ThreadSafeBlockAllocator::alloc(size_t size);
    void ThreadSafeBlockAllocator::free(void *ptr);
};
