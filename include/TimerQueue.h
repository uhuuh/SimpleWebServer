#pragma once
#include "type.h"

class TimerQueue {
public:
    using TimeMap = std::multimap<TimeStamp, std::unique_ptr<Timer>>;
    using TimeMapIter = TimeMap::iterator;
    using IdMap = std::map<TimerId, TimeMapIter>;

    TimerQueue(Eventloop* loop);
    ~TimerQueue();
    TimerId addTimer(Callback cb, TimeStamp at, TimeStamp interval);
    void removeTimer(TimerId id);
    void _handleRead();

    Eventloop* m_loop;
    fd_t m_fd;

    std::unique_ptr<Channel> m_channel;
    TimeMap m_timeMap;
    IdMap m_idMap;
};
