#pragma once

class Interval
{
    public:
        double m_interval_time;
        double m_target_time;

    public:
        Interval();
        Interval(float seconds);

        void set_seconds(float seconds);
        void set_frequency(float hz);
        bool check();
        bool check_and_decrement();
        bool check_and_reset();
        unsigned int decrement_all();
        void reset();
};