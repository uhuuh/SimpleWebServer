#pragma once
#include "base.h"


class EventloopPool: public noncopyable {
public:
    explicit EventloopPool(int n_thread);
    ~EventloopPool();
    Eventloop* getLoop();
private:
    vector<unique_ptr<Eventloop>> loop_list;
    vector<thread> thread_list;
    mutex mu;
    condition_variable cv;
    const int n_thread;
    int next_i;
};

