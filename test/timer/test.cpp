#include "Eventloop.hpp"
#include "TimerScheduler.hpp"
#include "base.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include <stdio.h>
#include <iostream>


void test_timer_wheel() {
    TimerWheel ti;
    Callback cb = [] () { 
        auto now_ms = get_now_timestamp();
        printf("trigger_timer %lu\n", now_ms);
    };
    printf("add_timer %lu\n", get_now_timestamp());
    ti.add_timer({1000, cb});
    ti.add_timer({1000, cb});
    ti.add_timer({2000, cb});
    ti.add_timer({2000, cb});

    Callback cb_time = [&] () {
        cout << "cb_start" << endl;

        int wait_ms = 5 * 1000;
        int acc_ms = 0;

        while (acc_ms <= wait_ms) {
            auto now_ms = get_now_timestamp();
            auto after_ms = ti.trigger_timer(now_ms);
            acc_ms += after_ms;
            this_thread::sleep_for(chrono::milliseconds(after_ms));
        }
    };

    cout << "timer_test" << endl;

    thread th(cb_time);
    th.join();

    cout << "timer_end" << endl;
}

void test_eventloop() {
    EventLoop loop;
    Callback cb = [] () { 
        auto now_ms = get_now_timestamp();
        printf("trigger_timer %lu\n", now_ms);
    }; 
    Callback cb2 = [&] () {
        loop.stop();
    };

    printf("start at %lu\n", get_now_timestamp());

    loop.add_timer({1000, cb});
    loop.add_timer({3000, cb});
    loop.add_timer({5000, cb2}); 

    loop.run();
};


int main() {
    test_eventloop();
    return 0;
}