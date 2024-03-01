#pragma once
#include <functional>

using TimeStamp = uint64_t;
using Callback = std::function<void()>;

struct Timer {
    Callback cb;
    TimeStamp at;
    TimeStamp interval;
};


using TimerId = Timer*;
using fd_t = int;

TimeStamp getNowTimeStamp();
fd_t createTimeFd();
void readTimeFd(fd_t fd);
void resetTimeFd(fd_t fd, uint64_t at);