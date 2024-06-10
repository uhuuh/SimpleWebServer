#pragma once
#include <cstdint>
#include "base.hpp"

class EventLoop;

class TimerQueue {
public:
    struct TimerId {
        uint64_t ms;
        uint64_t seq;
        void* timer;
    };

    TimerQueue(EventLoop *loop);
    ~TimerQueue();
    TimerId add_timer(Callback cb, uint64_t delay_ms);
    void cannel_timer(TimerId id);
private:
    class Impl;
    Impl* impl;
};

