#pragma once
#include "base.hpp"
#include "TimerScheduler.hpp"
#include "Eventloop.hpp"


class TimerSchedulerAdapter {
public:
    TimerId add_timer(Timer);
    void remove_timer(TimerId);
    TimerSchedulerAdapter(EventLoop* loop);
private:
    unique_ptr<TimerScheduler> ti_sche;
    EventLoop* loop;
    unique_ptr<EventLoop::Channel> ch;
};