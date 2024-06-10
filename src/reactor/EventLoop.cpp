#include <cstdint>
#include <unistd.h>
#include <vector>
#include "base.hpp"
#include "Activater.hpp"
#include "TimerQueue.hpp"
#include "Poller.hpp"
#include "Eventloop.hpp"



class EventLoop::Impl {
public:
    class Channel;
    class Poller;
    enum class EventType;
    Impl(EventLoop* loop):
        loop(loop),
        thread_id(gettid()),
        poller(new Epoll()),
        activater(new Activater(loop)),
        timer_queue(new TimerQueue(loop)) {}
    ~Impl() {
        quit();
    }

    void run() {
        is_loop = true;

        while (is_loop) {
            poller->poll();

            std::vector<Callback> now_cb_list;
            {
                std::lock_guard lock(mu);
                now_cb_list.swap(cb_list); // 使用swap操作减少互斥锁持有的时间
            }
            for (auto& cb: now_cb_list) {
                cb();
            }
        }
    }
    void quit() {
        is_loop = false;
        activater->activate(); 
    }
    bool is_same_thread() {
        auto now_id = gettid();
        return now_id == thread_id;
    }
    EventLoop::Channel* get_channel(int fd) {
        return new EventLoop::Channel(loop, fd);
    }
    bool add_callback(Callback cb) {
        if (is_same_thread()) {
            add_callback_now(cb);
            return true;
        } else {
            add_callback_after(cb);
            return false;
        }
    }
    void add_callback_now(Callback cb) {
        assertm(is_same_thread());
        cb();
    }
    void add_callback_after(Callback cb) {
        {
            std::lock_guard lock(mu);
            cb_list.push_back(cb);
        }
    }
    TimerQueue::TimerId add_timer(Callback cb, uint64_t delay_ms) {
        return timer_queue->add_timer(cb, delay_ms);
    }
    void remove_timer(TimerQueue::TimerId id) {
        timer_queue->cannel_timer(id);
    }

    bool is_loop;
    const int thread_id;
    void update_channel(EventLoop::Channel* ch) {
        assertm(is_same_thread());
        poller->update_channel(ch);
    }
    void remove_channel(EventLoop::Channel* ch) {
        assertm(is_same_thread());
        poller->remove_channel(ch);
    }

    std::mutex mu;
    EventLoop* loop;
    unique_ptr<EventLoop::Poller> poller;
    unique_ptr<Activater> activater;
    unique_ptr<TimerQueue> timer_queue;
    vector<Callback> cb_list;
    static const uint32_t timeout_ms = 10;
};

EventLoop::EventLoop() {
    impl = new Impl(this);
}
EventLoop::~EventLoop() {
    delete impl;
}
void EventLoop::run() {
    impl->run();
}
void EventLoop::quit() {
    impl->quit();
}
bool EventLoop::is_same_thread() {
    return impl->is_same_thread();
}
EventLoop::Channel* EventLoop::get_channel(int fd) {
    return impl->get_channel(fd);
}
bool EventLoop::add_callback(Callback cb) {
    return impl->add_callback(cb);
}
void EventLoop::add_callback_now(Callback cb) {
    impl->add_callback_now(cb);
}
void EventLoop::add_callback_after(Callback cb) {
    impl->add_callback_after(cb);
}
TimerQueue::TimerId EventLoop::add_timer(Callback cb, uint64_t delay_ms) {
    return impl->add_timer(cb, delay_ms);
}
void EventLoop::remove_timer(TimerQueue::TimerId id) {
    impl->remove_timer(id);
}
void EventLoop::update_channel(EventLoop::Channel* ch) {
    impl->update_channel(ch);
}
void EventLoop::remove_channel(EventLoop::Channel* ch) {
    impl->remove_channel(ch);
}