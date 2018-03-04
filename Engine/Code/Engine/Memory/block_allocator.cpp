#include "Engine/Memory/block_allocator.h"

BlockAllocator::BlockAllocator(size_t bs)
    :free_list(nullptr)
{
    // must be at least the block pointer size
    block_size = Max(bs, sizeof(block_t));
}

BlockAllocator::~BlockAllocator()
{
}

void* BlockAllocator::alloc(size_t size)
{
    if(size > block_size) {
        return nullptr;
    }

    void *ptr = free_list;
    if(nullptr == ptr) {
        ptr = ::malloc(block_size);
        ++alloc_count;
    }
    else {
        free_list = free_list->next;
    }

    return ptr;
}

void BlockAllocator::free(void *ptr)
{
    if(nullptr == ptr) {
        return;
    }

    block_t *block = (block_t*)ptr;
    block->next = free_list;
    free_list = block;
}