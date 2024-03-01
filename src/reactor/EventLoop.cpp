#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "Activater.h"
#include "TimerQueue.h"

thread_local void* g_ptr = nullptr;

Eventloop::Eventloop():
    m_mutex(),
    m_poller(std::make_unique<Poller>()),
    m_callbacks()
{
    assertm(g_ptr == nullptr, "one thread one loop");
    g_ptr = this;
    // todo thread_local底层还需要进一步了解, 在threadeventloop中尽管只是将ptr指针传递出来, 一样的地址取thread_local变量能得到不同的值
    // note 所有thread_local只能用来检测这个线程是否已经创建了loop

    m_threadId = gettid();
    m_activater.reset(new Activater(this));
    m_timerQueue.reset(new TimerQueue(this));

    LOG_TRACE << "init " << m_threadId;
}

Eventloop::~Eventloop() {
    // loop应该有自己的状态机, 但是在多线程环境下, 状态机不应该通过常见的enum来实现, 而应该通过多个bool来实现
    assertm(!m_flagStartLoop);
    LOG_TRACE << "destroy " << m_threadId;
}

void Eventloop::assertSameThread() {
    assertm(_isSameThread(), "loop diff thread");
}

bool Eventloop::_isSameThread() {
    return m_threadId == gettid();
}

void Eventloop::loop() {
    assertSameThread();
    assertm(!m_flagStartLoop);
    m_flagStartLoop = true;

    LOG_TRACE<< "loop start";
    std::vector<Channel*> channels;
    while (m_flagStartLoop) {
        m_flagHandleEvent = true;
        channels.clear();
        m_poller->poll(m_pollWaitSec, &channels); 
        if (channels.size() == 0) {
            std::vector<fd_t> fds;
            std::string fd_text;
            for (auto& ptr : m_poller->m_channelMap) {
                if (ptr.first != m_activater->m_fd && ptr.first != m_timerQueue->m_fd) {
                    fds.push_back(ptr.first);
                    fd_text += std::to_string(ptr.first) + ", ";
                }
            }
            LOG_TRACE << "poll empty, " << "activater " << m_activater->m_fd << ", timerQueue " << m_timerQueue->m_fd << ", other [" << fd_text << "]";
        }
        for (auto& channel: channels) {
            channel->handleEvent();
        }
        m_flagHandleEvent = false;

        std::vector<Callback> cbs;
        {
            std::lock_guard lock(m_mutex);
            cbs.swap(m_callbacks); // 使用swap操作减少临界区
        }
        for (auto& cb: cbs) {
            cb();
        }
    }

    LOG_TRACE << "loop quit";
}

void Eventloop::run(Callback cb, bool after) {
    if (!after && _isSameThread()) {
        cb();
    } else {
        {
            std::lock_guard lock(m_mutex);
            m_callbacks.push_back(cb);
        }
        // 如果m_flagHandleEvent为真, 说明当前处于事件阶段, 下一个回调阶段就能处理添加的functor, 不需要再激活
        if (!_isSameThread() || m_flagHandleEvent) {
            m_activater->activate();
            LOG_TRACE << "run activate";
        }
    }
}

void Eventloop::quit() {
    // assertm(m_flagStartLoop); 可以不调用loop后直接调用quit
    m_flagStartLoop = false; // bool本身是atomic, 不需要加锁
    // 只要是不同的线程一定要调用activate确保下一次poll能立刻唤醒. 因为即便检测到现在在处理事件, 仍然可能loop线程执行过快直接到达poll
    if (!_isSameThread()) {
        m_activater->activate();
    }
    LOG_TRACE << "call quit";
}


void Eventloop::updateChannel(Channel* channel, bool after) {
    run([this, channel] {m_poller->updateChannel(channel);}, after);
    LOG_TRACE << "updateChannel, channel " << channel->m_fd;
}

void Eventloop::removeChannel(fd_t fd, bool after) {
    run([this, fd] {m_poller->removeChannel(fd);}, after);
    LOG_TRACE << "removeChannel, channel " << fd;
}

TimerId Eventloop::addTimerNow(Callback cb, int interval) {
    auto at = getNowTimeStamp();
    return m_timerQueue->addTimer(cb, at, interval);
}

TimerId Eventloop::addTimerAt(Callback cb, int at, int interval) {
    return m_timerQueue->addTimer(cb, at, interval);
}

TimerId Eventloop::addTimerAfter(Callback cb, int after, int interval) {
    auto at = getNowTimeStamp() + after;
    return m_timerQueue->addTimer(cb, at, interval);
}

void Eventloop::removeTimer(TimerId id) {
    m_timerQueue->removeTimer(id);
}
