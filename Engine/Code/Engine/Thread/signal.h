#pragma once 

#include "Engine/Thread/thread.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Signal
{
public:
    Signal();
    ~Signal();

    void signal_all();
    void wait();
    bool wait_for(unsigned int ms);

public:
    HANDLE os_event;
};