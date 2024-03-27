#include "TimerQueue.h"
#include "Channel.h"
#include "EventLoop.h"
#include <sys/timerfd.h>
#include <chrono>
#include <sys/timerfd.h>
#include <unistd.h>



TimeStamp getNowTimeStamp() {
    // auto now = std::chrono::system_clock::now();
    auto now = std::chrono::steady_clock::now();
    // 这里使用steady_clock, timerfd_create那里也要相应使用CLOCK_MONOTONIC
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return milliseconds.count();
}

void rest_timefd(int fd, TimeStamp at) {
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof(newValue));
    newValue.it_value.tv_sec = at / int(1e3);
    newValue.it_value.tv_nsec = (at % int(1e3)) * int(1e6);
    assertm(::timerfd_settime(fd, TFD_TIMER_ABSTIME, &newValue, nullptr) >= 0);
}

TimerQueue::TimerQueue(Eventloop* loop)
{
    this->fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    this->loop = loop;

    this->ch.reset(new Channel(loop, fd));
    this->ch->enableEvent(EventType::READ, [this](){this->handle_read();});
}

TimerQueue::~TimerQueue() {
    close(fd);
}

TimerId TimerQueue::addTimer(Callback cb, TimeStamp at) {
    if (this->ch->enableRead == false) {
        this->ch->enableEvent(EventType::READ, [this](){this->handle_read();});
    }
    
    bool flag_early = at < timer_map.begin()->first;
    auto p = timer_map.insert({at, cb});
    --p; // insert返回的迭代器指向插入元素的下个元素
    if (flag_early) {
        rest_timefd(fd, at);
    }
    return p;
}

void TimerQueue::cancelTimer(TimerId id) {
    // todo stl容器的并发情况
    timer_map.erase(id);
}

void TimerQueue::handle_read() {
    auto now = getNowTimeStamp();
    auto p = timer_map.begin();
    for ( ; p != timer_map.end(); ) {
        if (p->first >= now) {
            p->second();
            p = timer_map.erase(p);
        } else {
            rest_timefd(fd, p->first);
            return;
        }
    }
    this->ch->disableEvent(EventType::READ);
}

