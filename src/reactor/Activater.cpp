#include "Activater.h"
#include <sys/eventfd.h>
#include "Channel.h"
#include "EventLoop.h"


Activater::Activater(Eventloop* loop):
    m_fd(-1),
    m_channel(),
    m_loop(loop)
{
    m_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assertm(m_fd >= 0);

    m_channel.reset(new Channel(m_fd));
    m_channel->addEvent(EventType::READ, [this]{_handleRead();});

    loop->updateChannel(m_channel.get(), false);
    LOG_TRACE << "init " << m_loop->m_threadId;
}

Activater::~Activater() {
    close(m_fd);
    LOG_TRACE << "destroy " << m_loop->m_threadId;
}

void Activater::activate() {
    uint64_t one = 1;
    auto n = ::write(m_fd, &one, sizeof one);
    assertm(n == sizeof one);
    LOG_TRACE << "activate";
}

void Activater::_handleRead() {
    m_loop->assertSameThread();

    uint64_t one = 1;
    auto n = ::read(m_fd, &one, sizeof one);
    assertm(n == sizeof one);
}