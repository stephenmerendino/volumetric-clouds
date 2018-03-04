#pragma once

#include "Engine/Memory/base_allocator.h"
#include "Engine/Math/MathUtils.hpp"
#include <cstdlib>

class BlockAllocator : public BaseAllocator
{
    struct block_t
    {
        block_t *next;
    };

public:
    BlockAllocator(size_t bs);
    ~BlockAllocator();

    void* alloc(size_t size);
    void free(void *ptr);

public:
    size_t block_size;
    block_t *free_list;
    unsigned int alloc_count;
};