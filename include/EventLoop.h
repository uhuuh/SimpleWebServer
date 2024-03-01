#pragma once
#include "type.h"

class Eventloop: noncopyable {
public:
    Eventloop();
    ~Eventloop();

    void loop();
    void quit();
    void run(Callback cb, bool isNextPoll);
    TimerId addTimerNow(Callback cb, int interval);
    TimerId addTimerAt(Callback cb, int at, int interval);
    TimerId addTimerAfter(Callback cb, int after, int interval);
    void removeTimer(TimerId id);
    void updateChannel(Channel* channel, bool isNextPoll);
    void removeChannel(fd_t fd, bool isNextPoll);
    void assertSameThread();

    bool _isSameThread();

    std::mutex m_mutex;
    std::unique_ptr<Poller> m_poller;
    std::unique_ptr<Activater> m_activater;
    std::unique_ptr<TimerQueue> m_timerQueue;
    std::vector<Callback> m_callbacks;
    bool m_flagStartLoop = false;
    bool m_flagHandleEvent = false;
    inline static const uint32_t m_pollWaitSec = 10;
    uint32_t m_threadId = -1;
};