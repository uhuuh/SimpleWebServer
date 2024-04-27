#pragma once
#include "EventLoop.h"
#include "base.h"
#include "Channel.h"
#include <memory>
#include <mutex>
#include <queue>

struct Timer {
    TimerId id;
    bool stop = false;
    Callback cb;
};

int timer_comp(const Timer* x, const Timer* y);

class TimerQueue: public Channel {
public:
    TimerQueue(Eventloop *loop);
    TimerId addTimer(Callback cb, TimeStamp at);
    void cancelTimer(TimerId id);
private:
    TimerSeq timer_seq;
    priority_queue<Timer*, vector<Timer*>, decltype(&timer_comp)> timer_queue;
    mutex mu;
    void handle_read();
};
