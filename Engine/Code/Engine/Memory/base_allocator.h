#pragma once

class BaseAllocator 
{
public:
    virtual void* alloc(size_t) = 0;
    virtual void free(void*) = 0;

public:
    template <typename T, typename ...ARGS>
    T* create(ARGS ...args)
    {
        void *buffer = alloc(sizeof(T));
        return new (buffer) T(args...);
    }

    template <typename T>
    void destroy(T *obj)
    {
        obj->~T();
        free(obj);
    }
};