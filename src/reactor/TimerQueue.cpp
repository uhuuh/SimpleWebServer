#include "TimerQueue.h"

#include "Channel.h"
#include "EventLoop.h"


TimerQueue::TimerQueue(Eventloop* loop):
    m_loop(loop)
{
    m_fd = createTimeFd();
    m_channel.reset(new Channel(m_fd));
    m_channel->addEvent(EventType::READ, [this]{_handleRead();});
    m_loop->updateChannel(m_channel.get(), false);
    LOG_TRACE << "init " << m_loop->m_threadId;
}

TimerQueue::~TimerQueue() {
    ::close(m_fd);
    LOG_TRACE << "destroy " << m_loop->m_threadId;
}

TimerId TimerQueue::addTimer(Callback cb, TimeStamp at, TimeStamp interval) {
    auto ptr = new Timer{cb, at, interval};

    auto fun = [this, ptr]() {
        auto at = ptr->at;
        bool need_reset = m_timeMap.empty() ? true : at < m_timeMap.begin()->first;
        auto iter = m_timeMap.insert({at, std::unique_ptr<Timer>(ptr)});
        auto ptr = iter->second.get();
        auto insert_res = m_idMap.insert({ptr, iter});
        assertm(insert_res.second);

        if (need_reset) resetTimeFd(m_fd, at);
    };
    m_loop->run(fun, false);
    LOG_TRACE << "addTimer";
    return ptr;
}

void TimerQueue::removeTimer(TimerId id) {
    auto fun = [this, id] {
        auto ptr = m_idMap.find(id);
        if (ptr != m_idMap.end()) {
            m_timeMap.erase(ptr->second);
            m_idMap.erase(ptr);
        }
    };
    m_loop->run(fun, false);
    LOG_TRACE << "removeTimer " << id;
}

void TimerQueue::_handleRead() {
    m_loop->assertSameThread();

    readTimeFd(m_fd);
    auto time_now = getNowTimeStamp();
    auto iter_after_now = m_timeMap.upper_bound(time_now);
    auto iter_early = m_timeMap.begin();
    while (iter_early != m_timeMap.end() && iter_early->first >= iter_after_now->first) {
        iter_early->second->cb();

        if (iter_early->second->interval != 0) {
            auto ptr = iter_early->second.get();
            auto unique_ptr = std::move(iter_early->second);
            m_idMap.erase(ptr);
            iter_early = m_timeMap.erase(iter_early);

            ptr->at += ptr->interval;
            auto iter = m_timeMap.insert({ptr->at, std::move(unique_ptr)});
            auto insert_res = m_idMap.insert({ptr, iter});
            assertm(insert_res.second);
        } else {
            auto ptr = iter_early->second.get();
            m_idMap.erase(ptr);
            iter_early = m_timeMap.erase(iter_early);
        }
    }
    auto time_next = iter_after_now->first;
    resetTimeFd(m_fd, time_next);
}