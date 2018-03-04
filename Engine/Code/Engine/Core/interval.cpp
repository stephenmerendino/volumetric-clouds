#include "Engine/Core/interval.h"
#include "Engine/Engine.hpp"

Interval::Interval()
    :m_interval_time(0.0f)
    ,m_target_time(0.0f)
{
}

Interval::Interval(float seconds)
    :Interval()
{
    set_seconds(seconds);
}

void Interval::set_seconds(float seconds)
{
    m_interval_time = seconds;
    m_target_time = engine_get_current_frame_time_seconds() + seconds;
}

void Interval::set_frequency(float hz) 
{ 
    set_seconds(1.0f / hz); 
}

bool Interval::check()
{
    double current_time = engine_get_current_frame_time_seconds();
    return (current_time >= m_target_time);
}

bool Interval::check_and_decrement()
{
    if(check()) {
        m_target_time += m_interval_time;
        return true;
    }
    else {
        return false;
    }
}

bool Interval::check_and_reset()
{
    if(check()) {
        reset();
        return true;
    }
    else {
        return false;
    }
}

unsigned int Interval::decrement_all()
{
    unsigned int count = 0;
    while(check_and_decrement()) {
        ++count;
    }

    return count;
}

void Interval::reset()
{
    m_target_time = engine_get_current_frame_time_seconds() + m_interval_time;
}