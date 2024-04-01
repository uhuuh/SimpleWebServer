#include "EventloopPool.h"
#include "Eventloop.h"
#include <chrono>
#include <thread>


EventloopPool::EventloopPool(int n_thread):
    n_thread(n_thread),
    next_i(0),
    thread_list(n_thread),
    loop_list(n_thread)
{
    for (int i = 0; i < n_thread; ++i) {
        thread_list[i] = thread([this, i] () {
            loop_list[i] = make_unique<Eventloop>();
            loop_list[i]->run();
        });
    }

    while (true) {
        int acc = 0;
        for (int i = 0; i < n_thread; ++i) {
            if (loop_list[i] && loop_list[i]->is_loop) {
                acc += 1;
            }
        }
        if (acc == n_thread) break;

        this_thread::sleep_for(chrono::milliseconds(10));
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


