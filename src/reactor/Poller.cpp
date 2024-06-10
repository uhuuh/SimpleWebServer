#include <sys/epoll.h>
#include <set>
#include <vector>
#include "base.hpp"
#include "Eventloop.hpp"
#include "Poller.hpp"
using namespace std;



class Epoll::Impl {
public:
    Impl() {
        epoll_fd = epoll_create1(0);
        assertm(epoll_fd);
        // INFO(format("poller_init | fd: {}", epoll_fd));
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
        // INFO(format("add_channel | fd: {}, need_read: {}, need_write: {}", ch->fd, ch->read_enable, ch->write_enable)); 
    }
    void remove_channel(EventLoop::Channel* ch) {
        assertm(fd_set.find(ch->get_fd()) != fd_set.end());

        epoll_event ev;
        assertm(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ch->get_fd(), &ev) >= 0);

        fd_set.erase(ch->get_fd());

        // INFO(format("remove_channel | fd: {}, need_read: {}, need_write: {}", ch->fd, ch->enable_read, ch->enable_write));
    }
    void poll() {
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

        // auto info_f = [&] () {
        //     set<int> ac_fd;
        //     for (int i = 0; i < n_event; ++i) {
        //         EventLoop::Channel* ch = static_cast<EventLoop::Channel*>(event_list[i].data.ptr);
        //         ac_fd.insert(ch->get_fd());
        //     }
        //     string ret = format("fd_all: [{}], ac_fd: [{}]", join(fd_set, ", "), join(ac_fd, ", "));
        //     return ret;
        // };
        // INFO(format("poll | id: {}, {}", gettid(), info_f()));

        for (int i = 0; i < n_event; ++i) {
            EventLoop::Channel* ch = static_cast<EventLoop::Channel*>(event_list[i].data.ptr);

            if (event_list[i].events & EPOLLIN) ch->trigger_event(EventLoop::EventType::READ);
            if (event_list[i].events & EPOLLOUT) ch->trigger_event(EventLoop::EventType::WRITE);
            ch->handle_event(); // 这里调用不会影响event_list
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