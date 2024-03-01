#include "Timer.h"
#include <chrono>
#include <sys/timerfd.h>
#include <string.h>
#include "Logger.h"
#include <unistd.h>


TimeStamp getNowTimeStamp() {
    // auto now = std::chrono::system_clock::now();
    auto now = std::chrono::steady_clock::now();
    // 这里使用steady_clock, timerfd_create那里也要相应使用CLOCK_MONOTONIC
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return milliseconds.count();
}

fd_t createTimeFd() {
    // auto fd = ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    auto fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    assertm(fd >= 0);
    return fd;
}
void readTimeFd(fd_t fd) {
    uint64_t howmany;
    ssize_t n = ::read(fd, &howmany, sizeof howmany);
    assertm(n == sizeof(howmany));
}

void resetTimeFd(fd_t fd, uint64_t at) {
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof(newValue));
    newValue.it_value.tv_sec = at / int(1e3);
    newValue.it_value.tv_nsec = (at % int(1e3)) * int(1e6);
    assertm(::timerfd_settime(fd, TFD_TIMER_ABSTIME, &newValue, nullptr) >= 0);
}
