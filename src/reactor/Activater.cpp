#include "Activater.h"
#include <functional>
#include <memory>
#include <sys/eventfd.h>
#include <unistd.h>
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include "base.h"


Activater::Activater(Eventloop* loop): Channel(loop, eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))
{
    INFO(format("activater_init | fd: {}", fd));
    addEvent(EventType::READ, [this](){this->handle_read();});
}

void Activater::activate() {
    assertm(loop != nullptr);

    // 这里不需要做并发防护
    uint64_t one = 1;
    // todo 操作系统层面write和read是如何做到处理并发的
    auto n = write(fd, &one, sizeof one);
    assertm(n == sizeof one);
}

void Activater::handle_read() {
    assertm(loop->isInSameThread());

    uint64_t one = 1;
    auto n = ::read(fd, &one, sizeof one);
    assertm(n == sizeof one);
}
