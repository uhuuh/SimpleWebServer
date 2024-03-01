#include "ThreadPoolEventloop.h"
#include "EventLoop.h"
#include "ThreadEventloop.h"


ThreadPoolEventloop::ThreadPoolEventloop(Eventloop* loop, uint32_t n_thread):
    m_loop(loop),
    m_n_thread(n_thread),
    m_nextIdx(0)
{
    m_threads.reserve(m_n_thread);
    m_loops.reserve(m_n_thread);
    for (int i = 0; i < m_n_thread; ++i) {
        auto ptr = std::make_unique<ThreadEventloop>();
        m_loops.push_back(ptr->getLoop());
        m_threads.push_back(std::move(ptr));
    }
}

Eventloop* ThreadPoolEventloop::getLoop() {
    m_loop->assertSameThread();

    auto now_idx = ++m_nextIdx;
    return m_loops[now_idx];
}