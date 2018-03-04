#pragma once

#include "Engine/Thread/thread.h"

#include <vector>

template <typename ...ARGS>
class Event
{
    public:
        struct event_sub_t;
        typedef void (*cb_t)(event_sub_t* sub, ARGS...); // In-between callback
        typedef void (*cb_with_arg_t)(void*, ARGS...);   // Real callback

        struct event_sub_t
        {
            cb_t cb;
            void* secondary_cb;
            void* user_arg;
            void* calling_object;
        };

        std::vector<event_sub_t> m_subscriptions;

    public:
        Event()
        {
        }

        ~Event()
        {
            m_subscriptions.clear();
        }

        void subscribe(void* user_arg, cb_with_arg_t cb)
        {
            event_sub_t sub;
            sub.cb = function_with_argument_callback;
            sub.secondary_cb = (void*)cb;
            sub.user_arg = user_arg;
            sub.calling_object = nullptr;
            m_subscriptions.push_back(sub);
        }

        void unsubscribe(void* user_arg, void* user_cb)
        {
            for(unsigned int i = 0; i < m_subscriptions.size(); ++i){
                event_sub_t& cb = m_subscriptions[i];
                if((cb.user_arg == user_arg) && (cb.secondary_cb == user_cb)){
                    m_subscriptions.erase(m_subscriptions.begin() + i);
                    return;
                }
            }
        }

        void unsubscribe_by_argument(void* user_arg)
        {
            for(unsigned int i = 0; i < m_subscriptions.size(); ++i){
                event_sub_t& cb = m_subscriptions[i];
                if(cb.user_arg == user_arg){
                    m_subscriptions[i] = m_subscriptions.back();
                    m_subscriptions.pop_back();
                }
            }
        }

        template <typename T>
        void subscribe_method(void* user_arg, T* obj, void(T::*mcb)(void*, ARGS...))
        {
            event_sub_t sub;
            sub.cb = method_callback<T, decltype(mcb)>;
            sub.secondary_cb = *((void**)&mcb);
            sub.calling_object = obj;
            sub.user_arg = user_arg;
            m_subscriptions.push_back(sub);
        }

        template <typename T>
        void unsubscribe(void* user_arg, T* obj, void(T::*mcb)(void*, ARGS...))
        {
            for(unsigned int i = 0; i < m_subscriptions.size(); ++i){
                event_sub_t& cb = m_subscriptions[i];
                if((cb.user_arg == user_arg) && (cb.calling_object == (void*)obj) && (cb.secondary_cb == *((void**)&mcb))){
                    m_subscriptions[i] = m_subscriptions.back();
                    m_subscriptions.pop_back();
                }
            }
        }

        template <typename T>
        void unsubscribe_method(T* obj, void(T::*mcb)(void*, ARGS...))
        {
            for(unsigned int i = 0; i < m_subscriptions.size(); ++i){
                event_sub_t& cb = m_subscriptions[i];
                if((cb.calling_object == (void*)obj) && (cb.secondary_cb == *((void**)&mcb))){
                    m_subscriptions[i] = m_subscriptions.back();
                    m_subscriptions.pop_back();
                }
            }
        }

        template <typename T>
        void unsubscribe_object(T* obj, void(T::*mcb)(void*, ARGS...))
        {
            for(unsigned int i = 0; i < m_subscriptions.size(); ++i){
                event_sub_t& cb = m_subscriptions[i];
                if(cb.secondary_cb == *((void**)&mcb)){
                    m_subscriptions[i] = m_subscriptions.back();
                    m_subscriptions.pop_back();
                }
            }
        }

        void trigger(ARGS ...args)
        {
            for(unsigned int i = 0; i < m_subscriptions.size(); ++i){
                event_sub_t& sub = m_subscriptions[i];
                sub.cb(&sub, args...); 
            }
        }

    public:
        static void function_with_argument_callback(event_sub_t* sub, ARGS ...args);

        template <typename T, typename MCB>
        static void method_callback(event_sub_t* sub, ARGS ...args);
};

template <typename ...ARGS>
void Event<ARGS...>::function_with_argument_callback(event_sub_t* sub, ARGS ...args)
{
    cb_with_arg_t cb = (cb_with_arg_t)sub->secondary_cb;
    cb(sub->user_arg, args...);
}

template <typename ...ARGS>
template <typename T, typename MCB>
void Event<ARGS...>::method_callback(event_sub_t* sub, ARGS ...args)
{
    MCB mcb = *(MCB*)&(sub->secondary_cb);
    T* obj = (T*)(sub->calling_object);
    (obj->*mcb)(sub->user_arg, args...);
}