#pragma once

#include "Engine/Thread/thread.h"
#include "Engine/Thread/critical_section.h"
#include <queue>

template <typename T>
class ThreadSafeQueue
{
public:
    bool empty();
    void push(const T& v);
    bool pop(T* out);
    T    front();

private:
    std::queue<T>    m_queue;
    CriticalSection  m_lock;
};

template<typename T>
bool ThreadSafeQueue<T>::empty()
{
    SCOPE_LOCK(&m_lock);
    return m_queue.empty();
}

template<typename T>
void ThreadSafeQueue<T>::push(const T& v)
{
    SCOPE_LOCK(&m_lock);
    m_queue.push(v);
}

template<typename T>
bool ThreadSafeQueue<T>::pop(T* out)
{
    SCOPE_LOCK(&m_lock);

    if(m_queue.empty()){
        return false;
    }else{
        *out = m_queue.front();
        m_queue.pop();
        return true;
    }
}

template<typename T>
T ThreadSafeQueue<T>::front()
{
    SCOPE_LOCK(&m_lock);
    return m_queue.front();
}