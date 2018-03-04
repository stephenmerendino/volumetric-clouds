#pragma once

#include "Engine/Thread/thread.h"
#include "Engine/Thread/atomic.h"
#include "Engine/Profile/profiler.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// --------------------------------------------
// Thread naming
#define MS_VC_EXCEPTION      (0x406d1388)

#pragma pack(push, 8)
struct THREADNAME_INFO
{
    DWORD type;
    const char *name;
    DWORD thread_id;
    DWORD flags;
};
#pragma pack(pop)
// --------------------------------------------

struct thread_pass_data_t
{
    thread_cb cb;
    void *arg;
};

static DWORD WINAPI thread_entry_point_common(void *arg)
{
    thread_pass_data_t *pass_ptr = (thread_pass_data_t*)arg;

    pass_ptr->cb(pass_ptr->arg);
    delete pass_ptr;
    return 0;
}

thread_handle_t thread_create(thread_cb cb, void *data)
{
   thread_pass_data_t *pass = new thread_pass_data_t();
   pass->cb = cb;
   pass->arg = data;

   thread_handle_t th = (thread_handle_t)::CreateThread(nullptr,
                                                        0,                        
                                                        thread_entry_point_common,  
                                                        pass,
                                                        0,
                                                        NULL);

   return th;
}

void thread_sleep(unsigned int ms)
{
    ::Sleep(ms);
}

void thread_yield()
{
    ::SwitchToThread();
}

void thread_detach(thread_handle_t th)
{
    ::CloseHandle(th);
}

void thread_join(thread_handle_t th)
{
    ::WaitForSingleObject(th, INFINITE);
    ::CloseHandle(th);
}

thread_id_t thread_get_id()
{
    return (thread_id_t)(uintptr_t)::GetCurrentThreadId();
}

void thread_set_name(const char* name)
{
    if(nullptr == name){
        return;
    }

    thread_id_t id = thread_get_id();
    if (0 != id) {
        THREADNAME_INFO info;
        info.type = 0x1000;
        info.name = name;
        info.thread_id =  (DWORD)(uintptr_t)id;
        info.flags = 0;

        __try 
        {
            RaiseException( MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)(&info) );
        } 
        __except (EXCEPTION_CONTINUE_EXECUTION) 
        {
        }

        profiler_set_thread_name(id, name);
    }
}