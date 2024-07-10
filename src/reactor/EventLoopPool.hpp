#pragma once
#include "base.hpp"
#include <memory>
#include <thread>
#include <mutex>


class EventLoop;

class EventloopPool: public noncopyable {
public:
    explicit EventloopPool(int n_thread);
    ~EventloopPool();
    EventLoop* getLoop();
    void wait_stop();
    void stop();

    vector<unique_ptr<EventLoop>> loop_list;
    const int n_thread;
private:
    vector<thread> thread_list;
    mutex mu;
    int next_i;
};

