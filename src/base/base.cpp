#include "base.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <chrono>
using namespace std;


void assertm(bool res) {
    if (!res) {
        if (errno != 0) {
            throw std::runtime_error(strerror(errno));
        } else {
            throw std::runtime_error("assertm unkonw error");
        }
    }
}

void assertm(bool res, const char* error_msg) {
    if (!res) {
        throw std::runtime_error(error_msg);
    }
}

string to_format_str(const char* format, ...) {
    int buf_size = 1024;
    string buf;
    buf.resize(buf_size);

    va_list args;
    va_start(args, format);
    int add = vsnprintf(&buf[0], buf_size, format, args);
    va_end(args);

    return buf;
}

uint64_t get_now_timestamp() {
    // auto now = std::chrono::system_clock::now();
    auto now = std::chrono::steady_clock::now();
    // 这里使用steady_clock, timerfd_create那里也要相应使用CLOCK_MONOTONIC
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return milliseconds.count();
}