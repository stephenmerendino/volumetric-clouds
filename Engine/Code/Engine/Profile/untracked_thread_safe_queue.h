#pragma once

#pragma once

#include "Engine/Profile/mem_tracker.h"
#include "Engine/Thread/thread.h"
#include "Engine/Thread/critical_section.h"

template <typename T>
class UntrackedThreadSafeQueue
{
public:
    bool empty();
    void push(const T& v);
    bool pop(T* out);
    T    front();

private:
    struct node_t
    {
        T data;
        node_t* next = nullptr;
    };

private:
    CriticalSection  m_lock;
    node_t* m_front = nullptr;
    node_t* m_back = nullptr;

};

template<typename T>
bool UntrackedThreadSafeQueue<T>::empty()
{
    SCOPE_LOCK(&m_lock);
    return nullptr == m_front;
}

template<typename T>
void UntrackedThreadSafeQueue<T>::push(const T& v)
{
    SCOPE_LOCK(&m_lock);

    node_t* new_tail = mem_construct_untracked_object<node_t>();
    new_tail->data = v;
    new_tail->next = nullptr;

    if(nullptr == m_front){
        m_front = new_tail;
        m_back = new_tail;
    }else{
        m_back->next = new_tail;
        m_back = new_tail;
    }
}

template<typename T>
bool UntrackedThreadSafeQueue<T>::pop(T* out)
{
    SCOPE_LOCK(&m_lock);

    if(empty()){
        return false;
    }else{
        *out = m_front->data;
        node_t* old_front = m_front;
        m_front = old_front->next;

        if(nullptr == m_front){
            m_back = nullptr;
        }

        mem_destroy_untracked_object(old_front);
        return true;
    }
}

template<typename T>
T UntrackedThreadSafeQueue<T>::front()
{
    SCOPE_LOCK(&m_lock);
    return m_front->data;
}