#include "Engine/Profile/mem_tracker.h"
#include "Engine/Profile/callstack.h"
#include "Engine/Profile/profiler.h"
#include "Engine/Core/Console.hpp"
#include "Engine/Core/Log.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Config/build_config.h"
#include "Engine/Thread/thread.h"
#include "Engine/Thread/critical_section.h"

#pragma warning(disable:4505)

static size_t   s_live_alloc_size           = 0;
static int      s_live_alloc_count          = 0;

static int      s_current_frame_alloc_count = 0;
static int      s_current_frame_free_count  = 0;
static size_t   s_current_frame_alloc_size  = 0;

static int      s_last_frame_alloc_count    = 0;
static int      s_last_frame_free_count     = 0;
static size_t   s_last_frame_alloc_size     = 0;

static size_t   s_alloc_highwater_size      = 0;

static size_t   s_frame_number              = 0;
static size_t   s_frame_alloc_history[MEMORY_TRACKER_FRAME_HISTORY];

static CriticalSection* s_lock;

struct allocation_t
{
    size_t alloc_size;
    size_t frame_number;

    #if defined(TRACK_MEMORY) && (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
    allocation_t* next;
    allocation_t* prev;
    Callstack* callstack;
    size_t group_count;
    #endif
};

static allocation_t* s_head = nullptr;

#define MAX_BYTES_STRING_SIZE 64

COMMAND(memory_profile, "output current memory profile")
{
    char bytes_string[MAX_BYTES_STRING_SIZE];

    console_info("----Current Memory Profile----");
    console_info("Number of live allocations: %i", s_live_alloc_count);
    console_info("Total memory allocated: %s", bytes_to_string(bytes_string, MAX_BYTES_STRING_SIZE, s_live_alloc_size));
    console_info("Number of allocations last frame: %i", s_current_frame_alloc_count);
    console_info("Number of frees last frame: %i", s_current_frame_free_count);
    console_info("Memory allocated last frame: %s", bytes_to_string(bytes_string, MAX_BYTES_STRING_SIZE, s_current_frame_alloc_size));
    console_info("Memory allocation highwater mark: %s", bytes_to_string(bytes_string, MAX_BYTES_STRING_SIZE, s_alloc_highwater_size));
}

COMMAND(log_allocs_to_console, "[uint:start_frame, uint:end_frame] Print live allocation info in console")
{
    #if TRACK_MEMORY != TRACK_MEMORY_VERBOSE 
        console_error("Live allocations are only tracked in TRACK_MEMORY_VERBOSE mode. Try running in a debug configuration.");
        return;
    #else
        unsigned int start_frame = 0;
        unsigned int end_frame = UINT_MAX;

        if(!args.is_at_end()){
            start_frame = args.next_uint_arg();
        }

        if(!args.is_at_end()){
            end_frame = args.next_uint_arg();
        }

        mem_log_live_allocs(start_frame, end_frame, true);
    #endif
}

static void sort_allocs(allocation_t* out_allocs_sorted, int num_allocs)
{
    for(int i = 1; i < num_allocs; i++){
        allocation_t x = out_allocs_sorted[i];
        int j = i - 1;
        while(j >= 0 && out_allocs_sorted[j].alloc_size < x.alloc_size){
            out_allocs_sorted[j + 1] = out_allocs_sorted[j]; 
            j--;
        }
        out_allocs_sorted[j + 1] = x;
    }
}

static void advance_frame_history()
{
    for(int i = 1; i < MEMORY_TRACKER_FRAME_HISTORY; i++){
        s_frame_alloc_history[i - 1] = s_frame_alloc_history[i];
    }
    s_frame_alloc_history[MEMORY_TRACKER_FRAME_HISTORY - 1] = s_live_alloc_size;
}

#if defined(TRACK_MEMORY) && (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
static void insert_into_live_list(allocation_t* alloc_ptr)
{
    Callstack* cs = create_callstack(3);
    alloc_ptr->callstack = cs;

    if(nullptr == s_head){
        s_head = alloc_ptr;
        s_head->next = s_head;
        s_head->prev = s_head;
    }else{
        allocation_t* temp = s_head->next;

        s_head->next = alloc_ptr;
        alloc_ptr->prev = s_head;

        alloc_ptr->next = temp;
        temp->prev = alloc_ptr;
    }

    alloc_ptr->group_count = 1;
    alloc_ptr->frame_number = s_frame_number;
}
#endif

#if defined(TRACK_MEMORY) && (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
static void remove_from_live_list(allocation_t* alloc_ptr)
{
    if(alloc_ptr == s_head && s_head->next == s_head){
        s_head = nullptr;
    }else{
        allocation_t* next = alloc_ptr->next;
        allocation_t* prev = alloc_ptr->prev;

        next->prev = prev;
        prev->next = next;

        alloc_ptr->next = alloc_ptr;
        alloc_ptr->prev = alloc_ptr;

        if(s_head == alloc_ptr){
            s_head = next;
        }
    }

    destroy_callstack(alloc_ptr->callstack);
}
#endif

void* mem_untracked_alloc(const size_t size)
{
    return malloc(size);
}

static void* tracked_alloc(const size_t size)
{
    SCOPE_LOCK(s_lock);

    s_live_alloc_size += size;
    s_current_frame_alloc_size += size;

    s_live_alloc_count++;
    s_current_frame_alloc_count++;

    if(s_live_alloc_size > s_alloc_highwater_size){
        s_alloc_highwater_size = s_live_alloc_size;
    }

    allocation_t* ptr = (allocation_t*)mem_untracked_alloc(sizeof(allocation_t) + size);
    ptr->alloc_size = size;

    #if defined(TRACK_MEMORY) && (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
        insert_into_live_list(ptr);
    #endif

    ptr++;

    profiler_track_alloc(size);

    return ptr; 
}

void mem_untracked_delete(void* p)
{
    free(p);
}

static void tracked_delete(void* p)
{
    SCOPE_LOCK(s_lock);

    allocation_t* alloc_ptr = (allocation_t*)p;    
    alloc_ptr--;

    #if defined(TRACK_MEMORY) && (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
        remove_from_live_list(alloc_ptr);
    #endif

    s_live_alloc_size -= alloc_ptr->alloc_size;
    s_live_alloc_count--;
    s_current_frame_free_count++;

    profiler_track_free(alloc_ptr->alloc_size);

    mem_untracked_delete(alloc_ptr);
}

void* operator new(const size_t size)
{
    // Lazy init lock on first tracked alloc, gets deleted in shutdown
    if(nullptr == s_lock){
        CriticalSection* storage = (CriticalSection*)mem_untracked_alloc(sizeof(CriticalSection));
        s_lock = new (storage) CriticalSection();
    }

    #if defined(TRACK_MEMORY)
        return tracked_alloc(size);
    #else
        return mem_untracked_alloc(size);
    #endif
}

void* operator new[](const size_t size)
{
    // Lazy init lock on first tracked alloc, gets deleted in shutdown
    if(nullptr == s_lock){
        CriticalSection* storage = (CriticalSection*)mem_untracked_alloc(sizeof(CriticalSection));
        s_lock = new (storage) CriticalSection();
    }

    #if defined(TRACK_MEMORY)
        return tracked_alloc(size);
    #else
        return mem_untracked_alloc(size);
    #endif
}


void operator delete(void* p)
{
    #if defined(TRACK_MEMORY)
        tracked_delete(p);
    #else
        mem_untracked_delete(p);
    #endif
}

void operator delete[](void* p)
{
    #if defined(TRACK_MEMORY)
        tracked_delete(p);
    #else
        mem_untracked_delete(p);
    #endif
}

void mem_tracker_init()
{
    callstack_system_init();
}

void mem_tracker_tick()
{
    SCOPE_LOCK(s_lock);

    s_frame_number++;

    s_last_frame_alloc_count = s_current_frame_alloc_count;
    s_last_frame_free_count = s_current_frame_free_count;
    s_last_frame_alloc_size = s_current_frame_alloc_size;

    s_current_frame_alloc_count = 0;
    s_current_frame_free_count = 0;
    s_current_frame_alloc_size = 0;

    advance_frame_history();
}

void mem_tracker_shutdown()
{
    callstack_system_shutdown();
}

unsigned int mem_get_live_alloc_byte_size()
{
    return s_live_alloc_size;
}

unsigned int mem_get_live_alloc_count()
{
    return s_live_alloc_count;
}

unsigned int mem_get_highwater_alloc_byte_size()
{
    return s_alloc_highwater_size;
}

unsigned int mem_get_last_frame_alloc_count()
{
    return s_last_frame_alloc_count;
}

unsigned int mem_get_last_frame_free_count()
{
    return s_last_frame_free_count;
}

unsigned int mem_get_last_frame_alloc_byte_size()
{
    return s_last_frame_alloc_size;
}

size_t* mem_get_frame_alloc_history()
{
    return s_frame_alloc_history;
}

static size_t get_total_size_of_allocs(allocation_t* allocs, unsigned int num_allocs)
{
    size_t total_size = 0;
    for(unsigned int i = 0; i < num_allocs; i++){
        total_size += allocs[i].alloc_size;
    }
    return total_size;
}

#if defined(TRACK_MEMORY) && (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
static int filter_allocs_by_frame(allocation_t** out_filtered_allocs, unsigned int start_frame, unsigned int end_frame)
{
    int num_live_allocs = s_live_alloc_count;

    *out_filtered_allocs = (allocation_t*)mem_untracked_alloc(num_live_allocs * sizeof(allocation_t));
    memset(*out_filtered_allocs, 0, num_live_allocs * sizeof(allocation_t));

    allocation_t* cursor = s_head;
    int counter = 0;
    do{
        if(cursor->frame_number >= start_frame && cursor->frame_number <= end_frame){
            memcpy(&(*out_filtered_allocs)[counter], cursor, sizeof(allocation_t));
            (*out_filtered_allocs)[counter].next = nullptr;
            (*out_filtered_allocs)[counter].prev = nullptr;
            counter++;
        }

        cursor = cursor->next;
    }while(cursor != s_head);

    return counter;
}
#endif

#if defined(TRACK_MEMORY) && (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
static int group_allocs_by_callstack(allocation_t* allocs, unsigned int num_allocs)
{
    // do actual grouping, delete any non needed allocs
    int num_groups = num_allocs;
    for(unsigned int i = 0; i < num_allocs - 1; ++i){
        if(nullptr == allocs[i].callstack){
            continue;
        }

        for(unsigned int j = i + 1; j < num_allocs; j++){
            if(nullptr == allocs[j].callstack){
                continue;
            }

            if(allocs[i].callstack->hash == allocs[j].callstack->hash){
                allocs[i].alloc_size += allocs[j].alloc_size;
                allocs[i].group_count++;
                memset(&allocs[j], 0, sizeof(allocation_t));
                num_groups--;
            }
        }
    }

    // plug in holes in the array
    for(unsigned int i = 0; i < num_allocs; i++){
        if(nullptr == allocs[i].callstack){
            for(unsigned int j = i + 1; j < num_allocs; j++){
                if(allocs[j].callstack != nullptr){
                    allocs[i] = allocs[j];
                    memset(&allocs[j], 0, sizeof(allocation_t));
                    break;
                }
            }
        }
    }

    return num_groups;
}
#endif

#if defined(TRACK_MEMORY) && (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
static void log_allocs(allocation_t* allocs, size_t num_allocs, unsigned int total_num_leaked_allocs, unsigned int start_frame, unsigned int end_frame, bool to_engine_console)
{
    // get total size
    size_t total_size = get_total_size_of_allocs(allocs, num_allocs);

    char bytes_string[MAX_BYTES_STRING_SIZE];
    bytes_to_string(bytes_string, MAX_BYTES_STRING_SIZE, total_size);

    log_tagged_printf("memory", "%i Leaked Allocations. Start Frame: %u. End Frame: %u. Total: %s\n", total_num_leaked_allocs, start_frame, end_frame, bytes_string);

    callstack_line_t lines[256];
    for(unsigned int i = 0; i < num_allocs; ++i){
        bytes_to_string(bytes_string, MAX_BYTES_STRING_SIZE, allocs[i].alloc_size);

        log_tagged_printf("memory", "Group contained %i allocation(s). Frame %d. Total: %s", allocs[i].group_count, allocs[i].frame_number, bytes_string);

        unsigned int num_lines = callstack_get_lines(lines, 256, allocs[i].callstack);
        for(unsigned int line = 0; line < num_lines; line++){
            log_tagged_printf("memory", "%s(%u): %s", lines[line].filename, lines[line].line, lines[line].function_name);
        }

        log_tagged_printf("memory", "");
    }
}
#endif

void mem_log_live_allocs(unsigned int start_frame, unsigned int end_frame, bool to_engine_console)
{
    SCOPE_LOCK(s_lock);

    if(nullptr == s_head){
        return;
    }

    #if defined(TRACK_MEMORY) && (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
        int num_live_allocs = s_live_alloc_count;

        allocation_t* filtered_allocs;

        int num_filtered_allocs = filter_allocs_by_frame(&filtered_allocs, start_frame, end_frame);

        int num_grouped_allocs = group_allocs_by_callstack(filtered_allocs, num_filtered_allocs);

        sort_allocs(filtered_allocs, num_grouped_allocs);

        log_allocs(filtered_allocs, num_grouped_allocs, num_live_allocs, start_frame, end_frame, to_engine_console);

        free(filtered_allocs);
    #else
        log_tagged_printf("memory", "%s", "Memory tracking disabled. Please run verbose memory tracking to output data.\n");
    #endif
}