#include "Poller.h"
#include "Channel.h"
#include "Logger.h"
#include "base.h"
#include <asm-generic/errno-base.h>
#include <cstring>
#include <iterator>
#include <sys/epoll.h>
#include <unistd.h>


Poller::Poller()
{
    epoll_fd = epoll_create1(0);
    assertm(epoll_fd);
    INFO(format("poller_init | fd: {}", epoll_fd));
}

Poller::~Poller() {
    // 这里特殊，要自己关闭fd
    close(epoll_fd);
}

void Poller::addChannel(Channel* ch) {
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = 0;
    if (ch->enableRead) {
        ev.events |= EPOLLIN;
    }
    if (ch->enableWrite) {
        ev.events |= EPOLLOUT;
    }
    if (fd_set.find(ch->fd) == fd_set.end()) {
        assertm(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ch->fd, &ev) >= 0);
    } else {
        assertm(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ch->fd, &ev) >= 0);
    }

    fd_set.insert(ch->fd);
    event_list.resize(fd_set.size()); // 这里不会减小
    INFO(format("add_channel | fd: {}, need_read: {}, need_write: {}", ch->fd, ch->enableRead, ch->enableWrite));
}

void Poller::removeChannel(Channel *ch) {
    // assertm(fd_set.find(ch->fd) != fd_set.end());
    if (fd_set.find(ch->fd) == fd_set.end()) return;

    epoll_event ev;
    assertm(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ch->fd, &ev) >= 0);

    fd_set.erase(ch->fd);

    INFO(format("remove_channel | fd: {}, need_read: {}, need_write: {}", ch->fd, ch->enableRead, ch->enableWrite));
}

void Poller::poll() {
    // 当event_list扩容时，&event_list.front()的地址发生在epoll_wait之外，故epoll_wait不会有影响
    auto p = reinterpret_cast<struct epoll_event*>(&event_list.front());
    int n_event = epoll_wait(epoll_fd, p, event_list.size(), timeout_ms);
    if (n_event == -1) {
        switch (errno) {
            case EINTR: // todo
                return;
            default:
                assertm(-1);
        }
    }

    auto info_f = [&] () {
        set<int> ac_fd;
        for (int i = 0; i < n_event; ++i) {
            Channel* ch = static_cast<Channel*>(event_list[i].data.ptr);
            ac_fd.insert(ch->fd);
        }
        string ret = format("fd_all: [{}], ac_fd: [{}]", join(fd_set, ", "), join(ac_fd, ", "));
        return ret;
    };
    INFO(format("poll | id: {}, {}", gettid(), info_f()));

    for (int i = 0; i < n_event; ++i) {
        Channel* ch = static_cast<Channel*>(event_list[i].data.ptr);

        if (event_list[i].events & EPOLLIN) ch->activateRead = true;
        if (event_list[i].events & EPOLLOUT) ch->activateWrite = true;
        ch->handleEvent(); // 这里调用不会影响event_list
    }
}
