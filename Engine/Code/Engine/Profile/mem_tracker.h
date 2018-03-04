#pragma once

#include <limits.h>

void            mem_tracker_init();
void            mem_tracker_tick();
void            mem_tracker_shutdown();

unsigned int    mem_get_live_alloc_byte_size();
unsigned int    mem_get_live_alloc_count();

unsigned int    mem_get_last_frame_alloc_count();
unsigned int    mem_get_last_frame_free_count();
unsigned int    mem_get_last_frame_alloc_byte_size();

unsigned int    mem_get_highwater_alloc_byte_size();

size_t*         mem_get_frame_alloc_history();

void            mem_log_live_allocs(unsigned int start_frame = 0, unsigned int end_frame = UINT_MAX, bool to_engine_console = false);

void*           mem_untracked_alloc(const size_t size);
void            mem_untracked_delete(void* p);

template<typename T, typename ...ARGS>
T* mem_construct_untracked_object(ARGS... args)
{
    T* storage = (T*)mem_untracked_alloc(sizeof(T)); 
    return new (storage) T(args...);
}

template<typename T>
void mem_destroy_untracked_object(T* untracked_object)
{
    untracked_object->~T();
    mem_untracked_delete(untracked_object);
}