#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>


Channel::Channel(Eventloop* loop, int fd): loop(loop), fd(fd) {}

Channel::~Channel() {
    // todo Channel应该负责关闭close吗
    loop->removeChannel(this);
}

void Channel::enableEvent(EventType ev, Callback cb) {
    if (ev == EventType::READ) {
        this->enableRead = true;
        this->handle_read_cb = cb;
        // todo 如果loop是const，好像addChannel不能调用
        loop->addChannel(this);
    } else if (ev == EventType::WRITE) {
        this->enableWrite = true; 
        this->handle_write_cb = cb;
        loop->addChannel(this);
    } else {
        // todo 异常是如何实现，使用异常会带来怎样的开销
        throw;
    }
}

void Channel::disableEvent(EventType ev) {
    if (ev == EventType::READ) {
        this->enableRead = false;
        loop->updateChannel(this);
    } else if (ev == EventType::WRITE) {
        this->enableWrite = false;;
        loop->updateChannel(this);
    } else {
        throw;
    }
}

void Channel::handleEvent() {
    if (activateRead) {
        this->handle_read_cb();
        activateRead = false;
    }
    if (activateWrite) {
        this->handle_write_cb();
        activateWrite = false;
    }
}

