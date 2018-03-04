#include "Engine/Profile/thread_profile.h"
#include "Engine/Profile/profiler.h"
#include "Engine/Profile/mem_tracker.h"
#include "Engine/Profile/profiler_report.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/log.h"

ThreadSafeBlockAllocator* ThreadProfile::s_allocator = new ThreadSafeBlockAllocator(sizeof(profiler_node_t));

static void node_set_parent(profiler_node_t* node, profiler_node_t* parent)
{
    ASSERT_OR_DIE(nullptr != node, "Error: child node is null");
    ASSERT_OR_DIE(nullptr != parent, "Error: parent node is null");

    node->parent = parent;
    
    if(nullptr == parent->first_child){
        parent->first_child = node;
    }else{
        profiler_node_t* first_child = parent->first_child; 
        profiler_node_t* last_child = first_child->prev_sibling;

        first_child->prev_sibling = node;
        node->next_sibling = first_child;

        last_child->next_sibling = node;
        node->prev_sibling = last_child;
    }
}

// custom deleter used in shared_ptr
void delete_tree(profiler_node_t* root)
{
    if(nullptr == root->first_child){
        ThreadProfile::s_allocator->destroy(root);
        return;
    }

    profiler_node_t* cursor = root->first_child;
    do{
        profiler_node_t* next = cursor->next_sibling; 
        delete_tree(cursor);
        cursor = next;
    }while(cursor != root->first_child);

    ThreadProfile::s_allocator->destroy(root);
}

ThreadProfile::ThreadProfile(const thread_id_t& id, const char* name)
    :m_id(id)
    ,m_name(name)
    ,m_sample_count(0)
    ,m_active_node(nullptr)
    ,m_current_state(ThreadProfileState::RUNNING)
{
}

ThreadProfile::~ThreadProfile()
{
    for(int i = 0; i < PROFILER_FRAME_HISTORY; i++){
        m_saved_trees[i] = nullptr;
    }
}

ThreadProfile::ThreadProfile(const ThreadProfile& copy)
    :m_id(copy.m_id)
    ,m_name(copy.m_name)
    ,m_sample_count(copy.m_sample_count)
    ,m_active_node(copy.m_active_node)
    ,m_current_state(copy.m_current_state)
{
    memcpy(m_saved_trees, copy.m_saved_trees, sizeof(m_saved_trees));
}

ThreadProfile& ThreadProfile::operator=(const ThreadProfile& copy)
{
    m_id = copy.m_id;
    m_name = copy.m_name;
    m_sample_count = copy.m_sample_count;
    m_active_node = copy.m_active_node;
    m_current_state = copy.m_current_state;
    memcpy(m_saved_trees, copy.m_saved_trees, sizeof(m_saved_trees));
    return *this;
}

void ThreadProfile::push_node(const char* tag)
{
    SCOPE_LOCK(&m_lock);

    m_sample_count++;

    if(nullptr == m_active_node && ThreadProfileState::PAUSING == m_current_state){
        return;
    }

    if(ThreadProfileState::RESUMING == m_current_state){
        return;
    }

    if(ThreadProfileState::RUNNING == m_current_state || ThreadProfileState::RUNNING_SINGLE_FRAME == m_current_state){
        add_node_to_tree(tag);
    }
}

void ThreadProfile::pop_node()
{
    SCOPE_LOCK(&m_lock);

    m_sample_count--;

    if(nullptr == m_active_node && ThreadProfileState::PAUSING == m_current_state){
        return;
    }

    // If resuming, don't start back up until we are at the start of a tree
    if(ThreadProfileState::RESUMING == m_current_state){
        if(0 == m_sample_count){
            m_current_state = ThreadProfileState::RUNNING;
        }
        return;
    }

    if(ThreadProfileState::STEPPING == m_current_state){
        if(0 == m_sample_count){
            m_current_state = ThreadProfileState::RUNNING_SINGLE_FRAME;
        }
        return;
    }

    move_active_node_in_tree();

    if(nullptr == m_active_node && ThreadProfileState::RUNNING_SINGLE_FRAME == m_current_state){
        m_current_state = ThreadProfileState::PAUSING;
    }
}

void ThreadProfile::push_alloc(const size_t alloc_byte_size)
{
    if(nullptr == m_active_node){
        return;
    }

    SCOPE_LOCK(&m_lock);

    m_active_node->num_allocs++;
    m_active_node->bytes_allocated += alloc_byte_size;
}

void ThreadProfile::push_free(const size_t free_byte_size)
{
    if(nullptr == m_active_node){
        return;
    }

    SCOPE_LOCK(&m_lock);

    m_active_node->num_frees++;
    m_active_node->bytes_freed += free_byte_size;
}

std::shared_ptr<profiler_node_t> ThreadProfile::get_prev_frame()
{
    SCOPE_LOCK(&m_lock);
    return m_saved_trees[PROFILER_FRAME_HISTORY - 1];
}

std::shared_ptr<profiler_node_t> ThreadProfile::get_prev_frame(const char* root_tag)
{
    SCOPE_LOCK(&m_lock);

    for(int i = PROFILER_FRAME_HISTORY - 1; i >= 0; i--){
        if(nullptr == m_saved_trees[i]){
            break;
        }

        if(strcmp(m_saved_trees[i].get()->tag, root_tag) == 0){
            return m_saved_trees[i];
        }
    }

    return nullptr;
}

void ThreadProfile::pause()
{
    SCOPE_LOCK(&m_lock);
    m_current_state = ThreadProfileState::PAUSING;
}

void ThreadProfile::resume()
{
    SCOPE_LOCK(&m_lock);
    if(ThreadProfileState::PAUSING == m_current_state){
        m_current_state = ThreadProfileState::RESUMING;
    }
}

void ThreadProfile::step()
{
    SCOPE_LOCK(&m_lock);
    if(ThreadProfileState::PAUSING == m_current_state){
        m_current_state = ThreadProfileState::STEPPING;
    }
}

void ThreadProfile::tree_report_last_frame()
{
    ProfilerReport report(*this);
    report.create_tree_view();
    report.log();
}

void ThreadProfile::flat_report_last_frame()
{
    ProfilerReport report(*this);
    report.create_flat_view();
    report.sort_by_self_time();
    report.log();
}

float ThreadProfile::calc_last_frame_fps()
{
    double seconds_elapsed = calc_last_frame_time_seconds();
    if(0.0 == seconds_elapsed){
        return 0.0;
    }

    return (float)(1.0 / seconds_elapsed); 
}

float ThreadProfile::calc_last_frame_time_seconds()
{
    if(nullptr == m_saved_trees[PROFILER_FRAME_HISTORY - 1].get()){
        return 0.0f;
    }

    uint64_t elapsed = m_saved_trees[PROFILER_FRAME_HISTORY - 1]->end_counter - m_saved_trees[PROFILER_FRAME_HISTORY - 1]->start_counter;
    return (float)perf_counter_to_seconds(elapsed);
}

float ThreadProfile::calc_avg_fps()
{
    SCOPE_LOCK(&m_lock);

    double total_frame_times = 0.0;
    int frame_count = 0;
    for(int i = PROFILER_FRAME_HISTORY - 1; i >= 0; i--){
        if(nullptr == m_saved_trees[i].get()){
            break;
        }

        uint64_t elapsed = m_saved_trees[i]->end_counter - m_saved_trees[i]->start_counter;
        double seconds_elapsed = perf_counter_to_seconds(elapsed);

        total_frame_times += seconds_elapsed;
        frame_count++;
    }

    if(0 == frame_count){
        return 0;
    }

    total_frame_times /= frame_count;

    return (float)(1.0 / total_frame_times);
}

static int count_nodes(profiler_node_t* root)
{
    int num_children = 1;

    profiler_node_t* cursor = root->first_child;

    if(nullptr == cursor){
        return num_children;
    }

    do{
        num_children += count_nodes(cursor);
        cursor = cursor->next_sibling;
    }while(cursor != root->first_child);

    return num_children;
}

int ThreadProfile::calc_last_frame_sample_count()
{
    if(nullptr == m_saved_trees[PROFILER_FRAME_HISTORY - 1].get()){
        return 0;
    }

    return count_nodes(m_saved_trees[PROFILER_FRAME_HISTORY - 1].get());
}

void ThreadProfile::save_tree(profiler_node_t* root)
{
    ASSERT_OR_DIE(root != nullptr, "Error: root node is null");
    SCOPE_LOCK(&m_lock);

    for(int i = 0; i < PROFILER_FRAME_HISTORY - 1; i++){
        m_saved_trees[i] = m_saved_trees[i + 1]; 
    }

    m_saved_trees[PROFILER_FRAME_HISTORY - 1] = std::shared_ptr<profiler_node_t>(root, delete_tree);
}

void ThreadProfile::add_node_to_tree(const char* tag)
{
    profiler_node_t* node = s_allocator->create<profiler_node_t>();
    node->tag = tag;
    node->start_counter = get_current_perf_counter();
    node->next_sibling = node;
    node->prev_sibling = node;

    if(nullptr != m_active_node){
        node_set_parent(node, m_active_node);  
    }

    m_active_node = node;
}

void ThreadProfile::move_active_node_in_tree()
{
    ASSERT_OR_DIE(nullptr != m_active_node, "Error: Mismatch of pushes and pops in profiler");

    m_active_node->end_counter = get_current_perf_counter();

    if(nullptr == m_active_node->parent){
        save_tree(m_active_node);
    }

    m_active_node = m_active_node->parent;
}