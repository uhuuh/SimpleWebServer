#include <cstdint>
#include <cstring>
#include <sys/timerfd.h>
#include <cstdint>
#include <unistd.h>
#include "TimerSchedulerAdapter.hpp"
#include "TimerScheduler.hpp"
#include "base.hpp"
#include "Eventloop.hpp"


void reset_timefd(int fd, uint64_t after) {
    struct itimerspec newValue;
    memset(&newValue, 0, sizeof(newValue));
    newValue.it_value.tv_sec = after / int(1e3);
    newValue.it_value.tv_nsec = (after % int(1e3)) * int(1e6);

    assertm(::timerfd_settime(fd, 0, &newValue, nullptr) >= 0);
}

void read_timefd(int fd) {
    uint64_t howmany;
    ssize_t n = read(fd, &howmany, sizeof howmany);
    assertm(n == sizeof(howmany));
}

TimerSchedulerAdapter::TimerSchedulerAdapter(EventLoop* loop): loop(loop), ti_sche(new TimerWheel()), ch(loop->get_channel(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))) {
    auto cb = [this] () {
        read_timefd(ch->get_fd());
        auto after_ms = ti_sche->trigger_timer(get_now_timestamp());
        reset_timefd(ch->get_fd(), after_ms);
    };

    reset_timefd(ch->get_fd(), 1); // 这里如果设为0，不会激活
    ch->enable_event(EventLoop::EventType::READ, cb);
}

TimerId TimerSchedulerAdapter::add_timer(Timer ti) {
    return ti_sche->add_timer(ti);
}

void TimerSchedulerAdapter::remove_timer(TimerId id) {
    ti_sche->remove_timer(id);
}
