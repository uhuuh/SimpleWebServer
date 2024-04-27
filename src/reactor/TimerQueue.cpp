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

int timer_comp(const Timer* x, const Timer* y) {
    if (x->id.ms != y->id.ms) {
        return x->id.ms > y->id.ms;
    } else {
        return x->id.seq > y->id.seq;
    }
}

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

TimerQueue::TimerQueue(Eventloop *loop): Channel(loop, timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)), timer_queue(&timer_comp)
{
    INFO(format("timer_queue_init | fd: {}", fd));
    addEvent(EventType::READ, [this](){this->handle_read();});
    disableEvent(EventType::READ);
}

TimerId TimerQueue::addTimer(Callback cb, TimeStamp delay_ms) {
    lock_guard<mutex> guard(mu);

    auto at_ms = get_now_timestamp() + delay_ms;
    if (timer_queue.empty() || at_ms < timer_queue.top()->id.ms) {
        enableEvent(EventType::READ);
        reset_timefd(fd, at_ms);
    }

    TimerId now_id{at_ms, timer_seq++, nullptr};
    auto timer = new Timer{now_id, false, cb};
    now_id.timer = timer;

    timer_queue.push(timer);

    return now_id;
}

void TimerQueue::cancelTimer(TimerId id) {
    lock_guard<mutex> gurad(mu);

    auto min_ms = timer_queue.top()->id.ms;
    if (id.ms < min_ms) return;
    if (id.ms == min_ms && id.seq < timer_seq) return;

    id.timer->stop = true;

    INFO(format("cancel_timer | timer_ms: {}, timer_seq: {}", id.ms, id.seq));
}

void TimerQueue::handle_read() {
    assertm(loop->isInSameThread());

    read_timefd(fd);

    auto now_ms = get_now_timestamp();

    INFO(format("timer_activate | now: {}", now_ms));

    while (!timer_queue.empty()) {
        auto timer = timer_queue.top();
        if (now_ms >= timer->id.ms) {
            if (!timer->stop) {
                timer->cb();
            } 
            timer_queue.pop();
            delete timer; // 释放资源
        } else {
            reset_timefd(fd, timer->id.ms);
            return;
        }
    }

    disableEvent(EventType::READ);
}

