#pragma once
#include "base.hpp"


class EventLoop;

class EventloopPool: public noncopyable {
public:
    explicit EventloopPool(int n_thread);
    ~EventloopPool();
    EventLoop* getLoop();
private:
    vector<unique_ptr<EventLoop>> loop_list;
    vector<thread> thread_list;
    mutex mu;
    const int n_thread;
    int next_i;
};

