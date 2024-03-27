#pragma once
#include "base.h"


class TimerQueue: public noncopyable {
public:
    TimerQueue(Eventloop* loop);
    ~TimerQueue();
    TimerId addTimer(Callback cb, TimeStamp at);
    void cancelTimer(TimerId id);
private:
    Eventloop* loop;
    fd_t fd;
    unique_ptr<Channel> ch;
    std::multimap<TimeStamp, Callback> timer_map;
    void handle_read();
};
