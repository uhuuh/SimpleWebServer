#include <memory>
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
    void activate() {
        /*
        quit可能在其他线程调用，quit中调用activate，故activate方法应该是线程安全的
        并且activate不能加入回调列表中，因为activate本身是为了激活loop立刻处理回调列表
        这里函数只使用了局部变量和只读全局变量，应该是线程安全的
        */

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
        // 多次write，read时返回值取决于EFD_SEMAPHORE
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