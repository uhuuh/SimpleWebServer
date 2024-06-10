#include "base.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>

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