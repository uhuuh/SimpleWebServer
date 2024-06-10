#include <cstdint>
#include <cstring>
#include <mutex>
#include <queue>
#include <sys/timerfd.h>
#include <cstdint>
#include "TimerQueue.hpp"
#include "base.hpp"
#include "EventLoop.hpp"


uint64_t get_now_timestamp() {
    // auto now = std::chrono::system_clock::now();
    auto now = std::chrono::steady_clock::now();
    // 这里使用steady_clock, timerfd_create那里也要相应使用CLOCK_MONOTONIC
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return milliseconds.count();
}

void reset_timefd(int fd, uint64_t at) {
    struct itimerspec newValue;
    memset(&newValue, 0, sizeof(newValue));
    newValue.it_value.tv_sec = at / int(1e3);
    newValue.it_value.tv_nsec = (at % int(1e3)) * int(1e6);

    assertm(::timerfd_settime(fd, TIMER_ABSTIME, &newValue, nullptr) >= 0);
}

void read_timefd(int fd) {
    uint64_t howmany;
    ssize_t n = read(fd, &howmany, sizeof howmany);
    assertm(n == sizeof(howmany));
}

class TimerQueue::Impl {
public:
    Impl(EventLoop *loop): 
        loop(loop),
        fd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
        ch(loop->get_channel(fd)) 
    {
        // INFO(format("timer_queue_init | fd: {}", fd));
        auto cb = bind(&Impl::handle_read, this);
        ch->set_event(EventLoop::EventType::READ, cb);
        ch->disable_event(EventLoop::EventType::READ);
    }
    ~Impl() {
        close(fd);
    }
    TimerId add_timer(Callback cb, uint64_t delay_ms) {
        lock_guard<mutex> lock(mu);

        auto at_ms = get_now_timestamp() + delay_ms;
        if (timer_queue.empty() || at_ms < timer_queue.top()->id.ms) {
            ch->enable_event(EventLoop::EventType::READ);
            reset_timefd(fd, at_ms);
        }

        TimerId now_id{at_ms, timer_seq++, nullptr};
        auto timer = new Timer{now_id, false, cb};
        now_id.timer = timer;

        timer_queue.push(timer);

        return now_id;
    }
    void cannel_timer(TimerId id) {
        lock_guard<mutex> lock(mu);

        auto min_ms = timer_queue.top()->id.ms;
        if (id.ms < min_ms) return;
        if (id.ms == min_ms && id.seq < timer_seq) return;

        reinterpret_cast<Timer*>(id.timer)->stop = true;

        // INFO(format("cancel_timer | timer_ms: {}, timer_seq: {}", id.ms, id.seq));
    }
private:
    struct Timer {
        TimerId id;
        bool stop = false;
        Callback cb;
        bool operator<(const Timer& other) {
            if (this->id.ms != other.id.ms) {
                return this->id.ms > other.id.ms;
            } else {
                return this->id.seq > other.id.seq;
            }
        }
    };

    uint64_t timer_seq;
    EventLoop* loop;
    mutex mu;
    int fd;
    unique_ptr<EventLoop::Channel> ch;
    priority_queue<Timer*> timer_queue;

    void handle_read() {
        assertm(loop->is_same_thread());

        read_timefd(fd);

        auto now_ms = get_now_timestamp();

        // INFO(format("timer_activate | now: {}", now_ms));

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

        ch->disable_event(EventLoop::EventType::READ);
    }
};

TimerQueue::TimerQueue(EventLoop* loop) {
    impl = new Impl(loop);
}
TimerQueue::~TimerQueue() {
    delete impl;
}
TimerQueue::TimerId TimerQueue::add_timer(Callback cb, uint64_t delay_ms) {
    return impl->add_timer(cb, delay_ms);
}
void TimerQueue::cannel_timer(TimerId id) {
    return impl->cannel_timer(id);
}