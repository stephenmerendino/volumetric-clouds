#pragma once

#include "Engine/Thread/thread.h"
#include "Engine/Config/build_config.h"
#include "Engine/Memory/thread_safe_block_allocator.h"

#include <memory>

enum class ThreadProfileState
{
    RUNNING,
    RUNNING_SINGLE_FRAME,
    PAUSING,
    STEPPING,
    RESUMING
};

struct profiler_node_t
{
    uint64_t            start_counter   = 0;
    uint64_t            end_counter     = 0;
    const char*         tag             = nullptr;

    profiler_node_t*    parent          = nullptr;
    profiler_node_t*    first_child     = nullptr;
    profiler_node_t*    next_sibling    = nullptr;
    profiler_node_t*    prev_sibling    = nullptr;

    size_t              num_allocs      = 0;
    size_t              num_frees       = 0;
    size_t              bytes_allocated = 0;
    size_t              bytes_freed     = 0;
};

class ThreadProfile
{
public:
    thread_id_t                         m_id;
    const char*                         m_name;
    int                                 m_sample_count;

    profiler_node_t*                    m_active_node;
    std::shared_ptr<profiler_node_t>    m_saved_trees[PROFILER_FRAME_HISTORY] = { 0 };

    CriticalSection                     m_lock;

    ThreadProfileState                  m_current_state;

    static ThreadSafeBlockAllocator*    s_allocator;

public:
    ThreadProfile(const thread_id_t& id, const char* name = nullptr);
    ~ThreadProfile();

    ThreadProfile(const ThreadProfile& copy);
    ThreadProfile& operator=(const ThreadProfile& copy);

    void push_node(const char* tag);
    void pop_node();
    void push_alloc(const size_t alloc_byte_size);
    void push_free(const size_t free_byte_size);

    std::shared_ptr<profiler_node_t> get_prev_frame();
    std::shared_ptr<profiler_node_t> get_prev_frame(const char* root_tag);

    void pause();
    void resume();
    void step();

    void tree_report_last_frame();
    void flat_report_last_frame();

    float calc_last_frame_fps();
    float calc_avg_fps();
    float calc_last_frame_time_seconds();
    int   calc_last_frame_sample_count();

private:
    void save_tree(profiler_node_t* root);
    void add_node_to_tree(const char* tag);
    void move_active_node_in_tree();
};