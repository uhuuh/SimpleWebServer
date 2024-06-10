#include "Logger.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <mutex>
#include <unistd.h>
using namespace std;
using namespace chrono;


int fun(size_t size) {
    // size = 1e8;
    // Logger::config("temp", true);

    int buf_size = 1000;
    char buf[buf_size];
    memset(buf, 'a', sizeof(buf));
    buf[buf_size - 1] = 0;

    mutex mu;
    size_t now_size = 0;

    auto cb = [&](){
        while (true) {
            LOG_TRACE("%s", buf);
            {
                lock_guard<mutex> lock(mu);
                now_size += buf_size;
                if (now_size >= size) break;
            }
        }
    };
    thread t1(cb);
    thread t2(cb);

    t1.join();
    t2.join();

    return 0;
}


int main() {
    mutex mu;
    mu.lock();
    mu.unlock();

    cout << "before" << endl;
    auto a = high_resolution_clock::now();

    size_t size = 1e9;
    fun(size);

    cout << "after" << endl;
    auto b = high_resolution_clock::now();
    auto c = duration_cast<milliseconds>(b - a).count();
    auto rate = static_cast<double>(size) / c * 1000;

    printf("size: %zu, time_sec: %f, rate_byte_per_sec: %f\n", size, static_cast<double>(c) / 1000, rate);

    return 0;
}



