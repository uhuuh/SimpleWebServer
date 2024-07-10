#pragma once
#include <cassert>
#include <cstdint>
#include <vector>
#include <list>
#include "base.hpp"


struct Timer {
    uint64_t trigger_after_ms;
    Callback cb;
};

struct TimerId {
    uint64_t trigger_at_ms = 0;
    void* it;
};

class TimerScheduler {
public:
    virtual TimerId add_timer(Timer) = 0;
    virtual void remove_timer(TimerId id) = 0;
    virtual uint64_t trigger_timer(uint64_t at_ms) = 0;
    virtual ~TimerScheduler() = default;
};


class TimerWheel: public TimerScheduler {
public:
    TimerWheel();
    TimerId add_timer(Timer timer) override;
    void remove_timer(TimerId id) override;
    uint64_t trigger_timer(uint64_t now_ms) override;
private:
    static const int every_ms = 10;
    static const int span_sec = 60;
    inline uint64_t get_slot_timestamp(uint64_t a=-1);
    vector<list<Timer>> wheel;
    using wheel_timer_it = list<Timer>::iterator;
    int slot_ptr = 0;
    uint64_t trigger_before_ms;
};

