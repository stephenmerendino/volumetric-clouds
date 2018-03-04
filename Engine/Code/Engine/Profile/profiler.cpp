#include "Engine/Profile/profiler.h"
#include "Engine/Profile/profiler_report.h"
#include "Engine/Profile/mem_tracker.h"
#include "Engine/Profile/untracked_thread_safe_queue.h"
#include "Engine/Thread/thread.h"
#include "Engine/Thread/signal.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma warning(disable:4505)

static uint64_t s_perf_freq;

enum class ProfilerEventType
{
    PUSH,
    POP,
    ALLOC,
    FREE
};

struct profiler_event_t
{
    thread_id_t         thread_id;
    ProfilerEventType   event_type;
    const char*         tag             = nullptr;  // Used by push
    size_t              byte_size       = 0;        // Used by alloc and free
};

struct thread_profile_list_node_t
{
    ThreadProfile* thread_profile;
    thread_profile_list_node_t* next;
    thread_profile_list_node_t* prev;
};

static thread_handle_t                                  s_profiler_thread = nullptr;
static UntrackedThreadSafeQueue<profiler_event_t*>      s_event_queue;
static Signal*                                          s_event_signal;
static CriticalSection*                                 s_lock;
static bool                                             s_running = false;
static thread_profile_list_node_t*                      s_profile_list = nullptr;

ThreadProfile* find_thread_profile(const thread_id_t& thread_id)
{
    SCOPE_LOCK(s_lock);
    if(nullptr == s_profile_list){
        return nullptr;
    }

    ThreadProfile* found = nullptr;

    thread_profile_list_node_t* iter = s_profile_list;
    do{
        if(iter->thread_profile->m_id == thread_id){
            found = iter->thread_profile;
            break;
        }

        iter = iter->next;
    }while(iter != s_profile_list);
    
    return (nullptr != found) ? found : nullptr;
}

ThreadProfile* create_thread_profile(const thread_id_t& thread_id)
{
    SCOPE_LOCK(s_lock);
    ThreadProfile* new_profile = mem_construct_untracked_object<ThreadProfile>(thread_id);

    thread_profile_list_node_t* new_list_node = mem_construct_untracked_object<thread_profile_list_node_t>();
    new_list_node->thread_profile = new_profile;

    if(nullptr == s_profile_list){
        new_list_node->next = new_list_node;
        new_list_node->prev = new_list_node;
        s_profile_list = new_list_node;
    }else{
        thread_profile_list_node_t* last_sibling = s_profile_list->prev;

        last_sibling->next = new_list_node;
        new_list_node->prev = last_sibling;

        new_list_node->next = s_profile_list;
        s_profile_list->prev = new_list_node;
    }

    return new_profile;
}

static void destroy_thread_profile_list(thread_profile_list_node_t* head)
{
    if(head == head->next){
        mem_destroy_untracked_object(head->thread_profile);
        mem_destroy_untracked_object(head);
        return;
    }

    thread_profile_list_node_t* next = head->next;
    thread_profile_list_node_t* prev = head->prev;

    next->prev = prev;
    prev->next = next;

    mem_destroy_untracked_object(head->thread_profile);
    mem_destroy_untracked_object(head);

    destroy_thread_profile_list(next);
}

ThreadProfile* find_or_create_thread_profile(const thread_id_t& thread_id)
{
    SCOPE_LOCK(s_lock);
    ThreadProfile* found = find_thread_profile(thread_id);
    if(nullptr != found){
        return found;
    }

    return create_thread_profile(thread_id);
}

static void profiler_handle_push_event(const thread_id_t& thread_id, const char* tag)
{
    ThreadProfile* thread_profile = find_or_create_thread_profile(thread_id);
    thread_profile->push_node(tag);
}

static void profiler_handle_pop_event(const thread_id_t& thread_id)
{
    ThreadProfile* thread_profile = find_or_create_thread_profile(thread_id);
    thread_profile->pop_node();
}

static void profiler_handle_alloc_event(const thread_id_t& thread_id, const size_t alloc_byte_size)
{
    ThreadProfile* thread_profile = find_or_create_thread_profile(thread_id);
    thread_profile->push_alloc(alloc_byte_size);
}

static void profiler_handle_free_event(const thread_id_t& thread_id, const size_t free_byte_size)
{
    ThreadProfile* thread_profile = find_or_create_thread_profile(thread_id);
    thread_profile->push_free(free_byte_size);
}

static void profiler_handle_event(profiler_event_t* event)
{
    switch(event->event_type){
        case ProfilerEventType::PUSH:   profiler_handle_push_event(event->thread_id, event->tag);           break;
        case ProfilerEventType::POP:    profiler_handle_pop_event(event->thread_id);                        break;
        case ProfilerEventType::ALLOC:  profiler_handle_alloc_event(event->thread_id, event->byte_size);    break;
        case ProfilerEventType::FREE:   profiler_handle_free_event(event->thread_id, event->byte_size);     break;
    }

    mem_destroy_untracked_object(event);
}

static void profiler_handle_events()
{
    profiler_event_t* event = nullptr;
    while(s_event_queue.pop(&event)){
        profiler_handle_event(event);
    }
}

uint64_t get_current_perf_counter()
{
    uint64_t counter;
    QueryPerformanceCounter((LARGE_INTEGER*)&counter);
    return counter;
}

double perf_counter_to_seconds(uint64_t counter)
{
    static bool get_freq = false;
    if(!get_freq){
        QueryPerformanceFrequency((LARGE_INTEGER*)&s_perf_freq);
        get_freq = true;
    }

    return (double)counter / (double)s_perf_freq;
}

#if defined(PROFILED_BUILD)

void main_profiler_thread(void* data)
{
    thread_set_name("Profiler");

    s_running = true;

    while(s_running){
        s_event_signal->wait();
        profiler_handle_events();
    }
}

void profiler_init()
{
    if(nullptr == s_event_signal){
        s_event_signal = mem_construct_untracked_object<Signal>();
    }

    if(nullptr == s_lock){
        s_lock = mem_construct_untracked_object<CriticalSection>();
    }


    s_profiler_thread = thread_create(main_profiler_thread, nullptr);

    profiler_set_thread_name(thread_get_id(), "Main");
}

void profiler_shutdown()
{
    s_running = false;
    s_event_signal->signal_all();
    thread_join(s_profiler_thread);
    destroy_thread_profile_list(s_profile_list);
    mem_destroy_untracked_object(s_event_signal);
}

void profiler_set_thread_name(const thread_id_t& id, const char* name)
{
    if(nullptr == s_lock){
        s_lock = mem_construct_untracked_object<CriticalSection>();
    }

    ThreadProfile* profile = find_or_create_thread_profile(id);
    profile->m_name = name;
}

void profiler_push(const char* tag)
{
    if(!s_running){
        return;
    }

    profiler_event_t* new_event = mem_construct_untracked_object<profiler_event_t>();
    new_event->event_type = ProfilerEventType::PUSH;
    new_event->tag = tag;
    new_event->thread_id = thread_get_id();

    s_event_queue.push(new_event);

    s_event_signal->signal_all();
}

void profiler_pop()
{
    if(!s_running){
        return;
    }

    profiler_event_t* new_event = mem_construct_untracked_object<profiler_event_t>();
    new_event->event_type = ProfilerEventType::POP;
    new_event->thread_id = thread_get_id();

    s_event_queue.push(new_event);

    s_event_signal->signal_all();
}

void profiler_track_alloc(size_t byte_size)
{
    if(!s_running){
        return;
    }

    profiler_event_t* new_event = mem_construct_untracked_object<profiler_event_t>();
    new_event->event_type = ProfilerEventType::ALLOC;
    new_event->thread_id = thread_get_id();
    new_event->byte_size = byte_size;

    s_event_queue.push(new_event);

    s_event_signal->signal_all();
}

void profiler_track_free(size_t byte_size)
{
    if(!s_running){
        return;
    }

    profiler_event_t* new_event = mem_construct_untracked_object<profiler_event_t>();
    new_event->event_type = ProfilerEventType::FREE;
    new_event->thread_id = thread_get_id();
    new_event->byte_size = byte_size;

    s_event_queue.push(new_event);

    s_event_signal->signal_all();
}

std::shared_ptr<profiler_node_t> profiler_get_prev_frame()
{
    thread_id_t id = thread_get_id();
    return profiler_get_prev_frame_for_thread(id);
}

std::shared_ptr<profiler_node_t> profiler_get_prev_frame(const char* root_tag)
{
    thread_id_t id = thread_get_id();
    return profiler_get_prev_frame_for_thread(id, root_tag);
}

std::shared_ptr<profiler_node_t> profiler_get_prev_frame_for_thread(const thread_id_t& id)
{
    ThreadProfile* profile = find_or_create_thread_profile(id);
    return profile->get_prev_frame();
}

std::shared_ptr<profiler_node_t> profiler_get_prev_frame_for_thread(const thread_id_t& id, const char* root_tag)
{
    ThreadProfile* profile = find_or_create_thread_profile(id);
    return profile->get_prev_frame(root_tag);
}

void profiler_pause_all()
{
    thread_profile_list_node_t* cursor = s_profile_list;
    do{
        cursor->thread_profile->pause();
        cursor = cursor->next;
    }while(cursor != s_profile_list);
}

void profiler_pause_thread(const thread_id_t& id)
{
    ThreadProfile* profile = find_or_create_thread_profile(id);
    profile->pause();
}

void profiler_resume_all()
{
    thread_profile_list_node_t* cursor = s_profile_list;
    do{
        cursor->thread_profile->resume();
        cursor = cursor->next;
    }while(cursor != s_profile_list);
}

void profiler_resume_thread(const thread_id_t& id)
{
    ThreadProfile* profile = find_or_create_thread_profile(id);
    profile->resume();
}

void profiler_step_all()
{
    thread_profile_list_node_t* cursor = s_profile_list;
    do{
        cursor->thread_profile->step();
        cursor = cursor->next;
    }while(cursor != s_profile_list);
}

void profiler_step_thread(const thread_id_t& id)
{
    ThreadProfile* profile = find_or_create_thread_profile(id);
    profile->step();
}

void profiler_tree_report_last_frame_all()
{
    thread_profile_list_node_t* cursor = s_profile_list;
    do{
        cursor->thread_profile->tree_report_last_frame();
        cursor = cursor->next;
    }while(cursor != s_profile_list);
}

void profiler_tree_report_last_frame_thread(const thread_id_t& id)
{
    ThreadProfile* profile = find_or_create_thread_profile(id);
    profile->tree_report_last_frame();
}

void profiler_flat_report_last_frame_all()
{
    thread_profile_list_node_t* cursor = s_profile_list;
    do{
        cursor->thread_profile->flat_report_last_frame();
        cursor = cursor->next;
    }while(cursor != s_profile_list);
}

void profiler_flat_report_last_frame_thread(const thread_id_t& id)
{
    ThreadProfile* profile = find_or_create_thread_profile(id);
    profile->flat_report_last_frame();
}

std::vector<ThreadProfile*> profiler_get_all_threads_snapshot()
{
    std::vector<ThreadProfile*> thread_profiles;

    thread_profile_list_node_t* cursor = s_profile_list;
    do{
        ThreadProfile* copy = cursor->thread_profile;
        thread_profiles.push_back(copy);
        cursor = cursor->next;
    }while(cursor != s_profile_list);

    return thread_profiles;
}

COMMAND(profiler_pause_all, "Pauses the profiler")
{
    profiler_pause_all();
}

COMMAND(profiler_resume_all, "Resume the profiler")
{
    profiler_resume_all();
}

COMMAND(profiler_step_all, "Snapshot a single frame with the profiler")
{
    profiler_step_all();
}

COMMAND(profiler_tree_report_all, "Prints out tree report of all threads previous frame")
{
    profiler_tree_report_last_frame_all();
}

COMMAND(profiler_flat_report_all, "Prints out flat report of all threads previous frame")
{
    profiler_flat_report_last_frame_all();
}

#else

void profiler_init(){}
void profiler_shutdown(){}
void profiler_set_thread_name(const thread_id_t& id, const char* name){}
void profiler_push(const char* tag){}
void profiler_pop(){}
void profiler_track_alloc(size_t byte_size){}
void profiler_track_free(size_t byte_size){}
void profiler_pause_all(){}
void profiler_pause_thread(const thread_id_t& id){}
void profiler_resume_all(){}
void profiler_resume_thread(const thread_id_t& id){}
void profiler_step_all(){}
void profiler_step_thread(const thread_id_t& id){}
std::shared_ptr<profiler_node_t> profiler_get_prev_frame(){ return nullptr; }
std::shared_ptr<profiler_node_t> profiler_get_prev_frame(const char* root_tag){ return nullptr; }
std::shared_ptr<profiler_node_t> profiler_get_prev_frame_for_thread(const thread_id_t& id){ return nullptr; }
std::shared_ptr<profiler_node_t> profiler_get_prev_frame_for_thread(const thread_id_t& id, const char* root_tag){ return nullptr; }
void profiler_tree_report_last_frame_all(){}
void profiler_tree_report_last_frame_thread(const thread_id_t& id){}
void profiler_flat_report_last_frame_all(){}
void profiler_flat_report_last_frame_thread(const thread_id_t& id){}
std::vector<ThreadProfile*> profiler_get_all_threads_snapshot(){ return std::vector<ThreadProfile*>(); }

#endif