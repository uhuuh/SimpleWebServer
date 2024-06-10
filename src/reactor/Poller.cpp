#include <sys/epoll.h>
#include <set>
#include <vector>
#include "Logger.hpp"
#include "base.hpp"
#include "Eventloop.hpp"
#include "Poller.hpp"
using namespace std;


class Epoll::Impl {
public:
    Impl() {
        epoll_fd = epoll_create1(0);
        assertm(epoll_fd);
    }
    ~Impl() {
        close(epoll_fd);
    }
    void update_channel(EventLoop::Channel* ch) {
        epoll_event ev;
        ev.data.ptr = ch;
        ev.events = 0;
        if (ch->is_enable(EventLoop::EventType::READ)) {
            ev.events |= EPOLLIN;
        }
        if (ch->is_enable(EventLoop::EventType::WRITE)) {
            ev.events |= EPOLLOUT;
        }
        if (fd_set.find(ch->get_fd()) == fd_set.end()) {
            assertm(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ch->get_fd(), &ev) >= 0);
        } else {
            assertm(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ch->get_fd(), &ev) >= 0);
        }

        fd_set.insert(ch->get_fd());
        event_list.resize(fd_set.size()); // 这里不会减小
        LOG_TRACE("poller update_channel, %s", ch->to_str().c_str());
    }
    void remove_channel(EventLoop::Channel* ch) {
        assertm(fd_set.find(ch->get_fd()) != fd_set.end());

        epoll_event ev;
        assertm(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ch->get_fd(), &ev) >= 0);

        fd_set.erase(ch->get_fd());

        LOG_TRACE("poller remove_channel, %s", ch->to_str().c_str());
    }
    void poll() {
        // 当event_list扩容时，&event_list.front()的地址发生在epoll_wait之外，故epoll_wait不会有影响
        auto p = reinterpret_cast<struct epoll_event*>(&event_list.front());
        int n_event = epoll_wait(epoll_fd, p, event_list.size(), timeout_ms);
        if (n_event > 0) {
            for (int i = 0; i < n_event; ++i) {
                EventLoop::Channel* ch = static_cast<EventLoop::Channel*>(event_list[i].data.ptr);

                if (event_list[i].events & EPOLLIN) ch->trigger_event(EventLoop::EventType::READ);
                if (event_list[i].events & EPOLLOUT) ch->trigger_event(EventLoop::EventType::WRITE);
                ch->handle_event(); // 这里调用不会影响event_list
            }
        } else if (n_event < 0) {
            switch (errno) {
                case EINTR:
                    return;
                    break;
                default:
                    assertm(-1);
            }
        } else {
            return;
        }
    }
private:
    int epoll_fd;
    set<int> fd_set;
    vector<epoll_event> event_list;
    static const int timeout_ms = 1000 * 3; 
};


Epoll::Epoll() {
    impl = new Impl();
}
Epoll::~Epoll() {
    delete impl;
}
void Epoll::update_channel(EventLoop::Channel* ch) {
    impl->update_channel(ch);
}
void Epoll::remove_channel(EventLoop::Channel* ch) {
    impl->remove_channel(ch);
}
void Epoll::poll() {
    impl->poll();
}