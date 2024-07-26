#include "EventloopPool.hpp"
#include "Eventloop.hpp"
#include <chrono>
#include <memory>
#include <thread>


EventloopPool::EventloopPool(int n_thread):
    n_thread(n_thread),
    next_i(0),
    thread_list(n_thread),
    loop_list(n_thread)
{
    for (int i = 0; i < n_thread; ++i) {
        loop_list[i] = make_unique<EventLoop>();
    }
}

EventloopPool::~EventloopPool() {
    for (int i = 0; i < n_thread; ++i) {
        loop_list[i]->stop();
    }
    wait_stop();
}

EventLoop* EventloopPool::getLoop() {
    auto now_i = (next_i + 1) % n_thread;
    return loop_list[now_i].get();
}

void EventloopPool::wait_stop() {
    for (int i = 0; i < n_thread; ++i) {
        if (thread_list[i].joinable()) {
            thread_list[i].join();
        }
    }
}

void EventloopPool::stop() {
    for (int i = 0; i < n_thread; ++i) {
        loop_list[i]->stop();
    }
}

void EventloopPool::start() {
    for (int i = 0; i < n_thread; ++i) {
        thread_list[i] = thread([this, i] () {
            loop_list[i]->run();
        });
    }
}