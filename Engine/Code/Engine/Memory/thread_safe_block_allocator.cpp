#include "Engine/Memory/thread_safe_block_allocator.h"

ThreadSafeBlockAllocator::ThreadSafeBlockAllocator(size_t bs)
    : free_list(nullptr)
{
    block_size = Max(bs, sizeof(block_t));
}

ThreadSafeBlockAllocator::~ThreadSafeBlockAllocator()
{
}

void* ThreadSafeBlockAllocator::alloc(size_t size)
{
    if(size > block_size) {
        return nullptr;
    }

    void *ptr = nullptr;

    {
        SCOPE_LOCK(&lock);
        if(free_list == nullptr) {
            ++alloc_count;
            return ::malloc(block_size);
        }
        else {
            ptr = free_list;
            free_list = free_list->next;
            return ptr;
        }
    }
}

void ThreadSafeBlockAllocator::free(void *ptr)
{
    if(nullptr != ptr) {
        SCOPE_LOCK(&lock);
        block_t *block = (block_t*)ptr;
        block->next = free_list;
        free_list = block;
    }
}
