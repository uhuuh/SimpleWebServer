#pragma once
#include "EventLoop.h"
#include "base.h"
#include "Channel.h"
#include <mutex>


class TimerQueue: public Channel {
public:
    TimerQueue(Eventloop *loop);
    TimerId addTimer(Callback cb, TimeStamp at);
    void cancelTimer(TimerId id);
private:
    multimap<TimeStamp, Callback> timer_map;
    mutex mu;
    void handle_read();
};
