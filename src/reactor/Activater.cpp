#include "Activater.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include "Channel.h"
#include "EventLoop.h"


Activater::Activater(Eventloop* loop): loop(loop)
{
    this->fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assertm(fd >= 0);

    this->ch.reset(new Channel(loop, fd));
    this->ch->enableEvent(EventType::READ, [this](){this->handle_read();});
}

Activater::~Activater() {
    // Channel最好不负责关闭fd
    close(fd);
}

void Activater::activate() {
    // note 只会被一个loop调用，不需要进行并发保护
    uint64_t one = 1;
    // todo 操作系统层面write和read是如何做到处理并发的
    auto n = ::write(fd, &one, sizeof one);
    assertm(n == sizeof one);
}

void Activater::handle_read() {
    uint64_t one = 1;
    auto n = ::read(fd, &one, sizeof one);
    assertm(n == sizeof one);
}
