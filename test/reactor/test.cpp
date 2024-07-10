#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>
#include "Eventloop.hpp"
#include "base.hpp"
#include <cstdio>
using namespace std;

void test_fd() {
    EventLoop loop;

    int fd_total = 5;
    vector<pair<int, int>> fd_list(fd_total);
    vector<unique_ptr<EventLoop::Channel>> ch_list(fd_total);
    for (int i = 0; i < fd_total; ++i) {
        assertm(pipe(reinterpret_cast<int*>(&fd_list[i])) >= 0);
        int read_fd = fd_list[i].first;
        ch_list[i] = loop.get_channel(read_fd);
        auto cb = [i, read_fd] () {
            char buf[1024];
            read(read_fd, buf, sizeof(buf));
            printf("No.%d fd %d read trigger: %s\n", i, read_fd, buf);
        };
        ch_list[i]->set_event(EventLoop::EventType::READ, cb);
    }
    
    auto thread_run = [&] () {
        while (!loop.is_run()) ;

        int count = 0;
        while (count < 3) {
            for (int i = 0; i < fd_total; ++i) {
                int write_fd = fd_list[i].second;
                const char* write_content = "xxx";
                write(write_fd, write_content, strlen(write_content));
            }

            count += 1;
            this_thread::sleep_for(chrono::seconds(1));
        }

        loop.stop();
    };


    thread t(thread_run);
    loop.run();
    t.join();
}

void test_time() {
    EventLoop loop;
    auto start = chrono::system_clock::now();
    auto dur_ms = 3000;
    auto cb = [&] () { 
        auto end = chrono::system_clock::now();
        loop.stop();
        auto real_dur_ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        cout << "timer trigger, after " << real_dur_ms << endl;
    };
    loop.add_timer({static_cast<uint64_t>(dur_ms), cb});

    loop.run();
}

int main() {
    cout << "test reactor" << endl;

    // test_time();
    test_fd();

    return 0;
}