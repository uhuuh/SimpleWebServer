#include "Poller.h"
#include "Channel.h"
#include <sys/epoll.h>



Poller::Poller()
{
    epoll_fd = epoll_create1(0);
    assertm(epoll_fd);
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
    assertm(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ch->fd, &ev));

    fd_set.insert(ch->fd);
    event_list.resize(fd_set.size());
}

void Poller::updateChannel(Channel* ch) {
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = 0;
    if (ch->enableRead) {
        ev.events |= EPOLLIN;
    }
    if (ch->enableWrite) {
        ev.events |= EPOLLOUT;
    }
    assertm(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ch->fd, &ev));
}

void Poller::removeChannel(Channel *ch) {
    epoll_event ev;
    assertm(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ch->fd, &ev));
}

void Poller::poll() {
    int n_event = epoll_wait(epoll_fd, &event_list.front(), event_list.size(), timeout_ms);
    for (int i = 0; i < n_event; ++i) {
        Channel* ch = static_cast<Channel*>(event_list[i].data.ptr);
        ch->handleEvent();
    }
}
