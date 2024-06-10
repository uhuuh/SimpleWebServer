#include <cstdint>
#include <memory>
#include <mutex>
#include <unistd.h>
#include <vector>
#include "base.hpp"
#include "Activater.hpp"
#include "TimerQueue.hpp"
#include "Poller.hpp"
#include "Eventloop.hpp"
using namespace std;


EventLoop::EventLoop():
    thread_id(gettid()),
    poller(new Epoll()),
    activater(new Activater(this)),
    timer_queue(new TimerQueue(this)) {}

EventLoop::~EventLoop() {
    stop();
}

void EventLoop::run() {
    is_loop = true;

    while (is_loop) {
        poller->poll();

        vector<Callback> now_cb_list;
        {
            lock_guard lock(mu);
            now_cb_list.swap(cb_list); // 使用swap操作减少互斥锁持有的时间
        }
        for (auto& cb: now_cb_list) {
            cb();
        }
    }
}

void EventLoop::stop() {
    is_loop = false;
    activater->activate(); 
}

bool EventLoop::is_same_thread() {
    auto now_id = gettid();
    return now_id == thread_id;
}

bool EventLoop::is_run() {
    return is_loop;
}

unique_ptr<EventLoop::Channel> EventLoop::get_channel(int fd) {
    return unique_ptr<EventLoop::Channel>(new EventLoop::Channel(this, fd));
}

bool EventLoop::add_callback(Callback cb) {
    if (is_same_thread()) {
        add_callback_now(cb);
        return true;
    } else {
        add_callback_after(cb);
        return false;
    }
}

void EventLoop::add_callback_now(Callback cb) {
    assertm(is_same_thread());
    cb();
}
void EventLoop::add_callback_after(Callback cb) {
    {
        lock_guard lock(mu);
        cb_list.push_back(cb);
    }
}
TimerQueue::TimerId EventLoop::add_timer(Callback cb, uint64_t delay_ms) {
    return timer_queue->add_timer(cb, delay_ms);
}
void EventLoop::remove_timer(TimerQueue::TimerId id) {
    timer_queue->remove_timer(id);
}

void EventLoop::update_channel(EventLoop::Channel* ch) {
    assertm(is_same_thread());
    poller->update_channel(ch);
}
void EventLoop::remove_channel(EventLoop::Channel* ch) {
    assertm(is_same_thread());
    poller->remove_channel(ch);
}

