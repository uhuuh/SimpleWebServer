#include <sys/eventfd.h>
#include <unistd.h>
#include "Activater.hpp"
#include "Eventloop.hpp"
#include "base.hpp"


class Activater::Impl {
public:
    Impl(EventLoop* loop) : 
        loop(loop), 
        fd(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
        ch(loop->get_channel(fd))
    {
        auto cb = bind(&Impl::handle_read, this);
        ch->set_event(EventLoop::EventType::READ, cb);
    }
    ~Impl() {
        close(fd);
    }
    void activate() {
        assertm(loop->is_same_thread());

        uint64_t one = 1;
        auto n = write(fd, &one, sizeof one);
        assertm(n == sizeof one);
    }
private:
    EventLoop* loop;
    int fd;
    unique_ptr<EventLoop::Channel> ch;

    void handle_read() {
        assertm(loop->is_same_thread());

        uint64_t one = 1;
        auto n = read(fd, &one, sizeof one);
        assertm(n == sizeof one);
    }
};

Activater::Activater(EventLoop* loop) {
    impl = new Impl(loop);
}
Activater::~Activater() {
    delete impl;
}
void Activater::activate() {
    impl->activate();
}