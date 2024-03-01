#include "ThreadEventloop.h"
#include "EventLoop.h"

Eventloop* ThreadEventloop::getLoop() {
    assertm(!m_flagHasLoop, "ThreadEventloop has one loop");
    m_flagHasLoop = true;

    m_thread.reset(new std::thread([this]{_runLoop();}));

    {
        std::unique_lock lock(m_mutex); // unique_lock可以手动解锁, 所以不使用lock_guard
        m_cv.wait(lock, [this]{return m_loop != nullptr;}); // 相当于内部也有一个while, 回调满足时退出
    }
    return m_loop;
}

void ThreadEventloop::_runLoop() {
    Eventloop loop;
    {
        std::lock_guard lock(m_mutex);
        m_loop = &loop;
        m_cv.notify_one();
    }
    loop.loop();
}

ThreadEventloop::~ThreadEventloop() {
    m_loop->quit();
    m_thread->join();
}