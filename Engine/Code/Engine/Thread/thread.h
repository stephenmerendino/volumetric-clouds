#pragma once

#include "Engine/Thread/critical_section.h"
#include "Engine/Core/Common.hpp"

#include <tuple>
#include <utility>

struct thread_handle_name_t {};
struct thread_id_name_t {};

typedef thread_handle_name_t* thread_handle_t;
typedef thread_id_name_t* thread_id_t;

typedef void (*thread_cb)(void*);

thread_handle_t     thread_create(thread_cb cb, void *data);
void                thread_sleep(unsigned int ms);
void                thread_yield();
void                thread_detach(thread_handle_t th);
void                thread_join(thread_handle_t th);
thread_id_t         thread_get_id();
void                thread_set_name(const char* name);

template <typename CB, typename ...ARGS>
struct pass_data_t
{
    CB cb;
    std::tuple<ARGS...> args;

    pass_data_t(CB cb, ARGS ...args)
        : cb(cb)
        , args(args...)
    {
    }
};

template<typename CB, typename TUPLE, size_t... INDICES>
void forward_thread_arguments_with_indices(CB cb, TUPLE& args, std::integer_sequence<size_t, INDICES...>)
{
    UNUSED(args);
    cb(std::get<INDICES>(args)...);
}

template<typename CB, typename... ARGS>
void forward_arguments_thread(void* ptr)
{
    pass_data_t<CB,ARGS...>* args = (pass_data_t<CB,ARGS...>*)ptr;
    forward_thread_arguments_with_indices(args->cb, args->args, std::make_index_sequence<sizeof...(ARGS)>());
    delete args;
}

template<typename CB, typename... ARGS>
thread_handle_t thread_create(CB entry_point, ARGS... args)
{
    pass_data_t<CB,ARGS...>* pass = new pass_data_t<CB,ARGS...>(entry_point, args...);
    return thread_create( forward_arguments_thread<CB, ARGS...>, (void*)pass);
}