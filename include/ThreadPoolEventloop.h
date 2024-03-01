#pragma once
#include "type.h"

class ThreadPoolEventloop: public noncopyable {
public:
    ThreadPoolEventloop(Eventloop* loop, uint32_t n_thread);
    Eventloop* getLoop();
    using EventLoopList = std::vector<Eventloop*>;
    using ThreadList = std::vector<std::unique_ptr<ThreadEventloop>>;
    Eventloop *m_loop;
    uint32_t m_n_thread;
    EventLoopList m_loops;
    ThreadList m_threads;
    int m_nextIdx;
};

