#include "EventloopPool.h"
#include "Eventloop.h"


EventloopPool::EventloopPool(int n_thread):
    n_thread(n_thread),
    next_i(0),
    thread_list(n_thread),
    loop_list(n_thread)
{
    for (int i = 0; i < n_thread; ++i) {
        auto loop = new Eventloop();
        loop_list[i].reset(loop);
        thread_list[i] = thread([loop]() {
            loop->run();
        });
    }

    while (true) {
        int n = 0;
        for (int i = 0; i < n_thread; ++i) {
            if (loop_list[i]->is_loop) {
                n += 1;
            }
        }
        if (n >= n_thread) break;  
    }
}

EventloopPool::~EventloopPool() {
    for (int i = 0; i < n_thread; ++i) {
        loop_list[i]->quit();
        thread_list[i].join();
    }
}

Eventloop* EventloopPool::getLoop() {
    auto now_i = (next_i + 1) % n_thread;
    return loop_list[now_i].get();
}


