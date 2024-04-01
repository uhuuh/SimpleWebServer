#include "TimerQueue.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include "base.h"
#include <sys/timerfd.h>
#include <chrono>
#include <mutex>
#include <sys/timerfd.h>
#include <unistd.h>



TimeStamp get_now_timestamp() {
    // auto now = std::chrono::system_clock::now();
    auto now = std::chrono::steady_clock::now();
    // 这里使用steady_clock, timerfd_create那里也要相应使用CLOCK_MONOTONIC
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return milliseconds.count();
}

void reset_timefd(int fd, TimeStamp at) {
    struct itimerspec newValue;
    memset(&newValue, 0, sizeof(newValue));
    newValue.it_value.tv_sec = at / int(1e3);
    newValue.it_value.tv_nsec = (at % int(1e3)) * int(1e6);

    assertm(::timerfd_settime(fd, TFD_TIMER_ABSTIME, &newValue, nullptr) >= 0);
}

void read_timefd(fd_t fd) {
    uint64_t howmany;
    ssize_t n = read(fd, &howmany, sizeof howmany);
    assertm(n == sizeof(howmany));
}

TimerQueue::TimerQueue(Eventloop *loop): Channel(loop, timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))
{
    INFO(format("timer_queue_init | fd: {}", fd));
    addEvent(EventType::READ, [this](){this->handle_read();});
    disableEvent(EventType::READ);
}

TimerId TimerQueue::addTimer(Callback cb, TimeStamp delay_ms) {
    lock_guard<mutex> guard(mu);

    auto at_ms = get_now_timestamp() + delay_ms;
    bool flag_early = timer_map.size() == 0 ? true : at_ms < timer_map.begin()->first;

    auto p = timer_map.insert({at_ms, cb});
    --p; // insert返回的迭代器指向插入元素的下个元素
    auto p2 = reinterpret_cast<long long*>(&p);
    auto id = *p2;

    if (flag_early) {
        reset_timefd(fd, at_ms);
    }

    if (enableRead == false) {
        enableEvent(EventType::READ);
    }

    INFO(format("add_timer | timer_id: {}, at: {}", id, at_ms));
    return id;
}

void TimerQueue::cancelTimer(TimerId id) {
    lock_guard<mutex> gurad(mu);

    using iter = decltype(timer_map)::iterator;
    auto pp = reinterpret_cast<iter*>(&id);
    auto p = *pp;

    timer_map.erase(p);

    INFO(format("cancel_timer | timer_id: {}", id));
}

void TimerQueue::handle_read() {
    assertm(loop->isInSameThread());

    read_timefd(fd);

    auto now_ms = get_now_timestamp();

    INFO(format("timer_activate | now: {}", now_ms));

    auto p = timer_map.begin();
    for ( ; p != timer_map.end(); ) {
        if (now_ms >= p->first) {
            p->second();
            p = timer_map.erase(p);
        } else {
            // 如果还有timer，重置一下timefd
            reset_timefd(fd, p->first);
            return;
        }
    }
    // 如果没有timer，关闭read
    disableEvent(EventType::READ);
}

