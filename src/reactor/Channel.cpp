#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include "base.h"
#include "easyloggingpp.h"
#include <cstddef>
#include <unistd.h>


Channel::Channel(Eventloop *loop, fd_t fd): loop(loop), fd(fd) 
{
    assertm(loop != nullptr);
    assertm(fd >= 0);
}

Channel::~Channel() {
    // 好像epoll清除fd时确保fd时完好的，故县epoll清除再close
    loop->removeChannel(this);
    close(fd);
}

void Channel::addEvent(EventType ev, Callback cb) {
    if (ev == EventType::READ) {
        this->enableRead = true;
        this->handle_read_cb = cb;
        // todo 如果loop是const，好像addChannel不能调用
    } else if (ev == EventType::WRITE) {
        this->enableWrite = true; 
        this->handle_write_cb = cb;
    } else {
        // todo 异常是如何实现，使用异常会带来怎样的开销
        throw;
    }
    loop->addChannel(this);
}

void Channel::enableEvent(EventType ev) {
    if (ev == EventType::READ) {
        if (!enableRead) {
            this->enableRead = true;
            loop->addChannel(this);
        }
    } else if (ev == EventType::WRITE) {
        if (!enableWrite) {
            this->enableWrite = true;
            loop->addChannel(this);
        }
    } else {
        throw;
    }
}

void Channel::disableEvent(EventType ev) {
    if (ev == EventType::READ) {
        if (enableRead) {
            this->enableRead = false;
            loop->addChannel(this);
        }   
    } else if (ev == EventType::WRITE) {
        if (enableWrite) {
            this->enableWrite = false;
            loop->addChannel(this);
        }
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

