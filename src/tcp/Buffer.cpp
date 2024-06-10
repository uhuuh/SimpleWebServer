#include "Buffer.hpp"
#include <unistd.h>


int Buffer::push_from(int fd) {
    auto remainRight = buf.capacity() - right_write_ptr;
    // todo 通过readv结果, 如果结果在栈空间上则自动扩容
    if (remainRight < init_capa / 2) {
        buf.resize(buf.capacity() * 2);
        remainRight = buf.capacity() - right_write_ptr;
    }
    auto n = ::read(fd, begin() + right_write_ptr, remainRight);
    if (n > 0) {
        right_write_ptr += n;
    }
    return n;
}

int  Buffer::pop_to(int fd) {
    auto n = ::write(fd, begin() + left_read_ptr, get_size());
    if (n > 0) {
        left_read_ptr += n;
    }
    return n;
}


int Buffer::get_size() {
    return right_write_ptr - left_read_ptr;
}

Buffer::Buffer():
        left_read_ptr(0),
        right_write_ptr(0),
        buf(init_capa)
{}

char *Buffer::begin() {
    return &(*buf.begin());
}

std::string_view Buffer::peek() {
    return {begin() + left_read_ptr, static_cast<unsigned long>(get_size())};
}

void Buffer::pop(int len) { // 弹出左边的
    auto n = peek().size();
    assertm(len >= 0 && len <= n);
    left_read_ptr += n;
}

void Buffer::pop() {
    auto n = peek().size();
    pop(n);
}

void Buffer::buf_move_head() {
    auto size = get_size();
    std::copy(buf.begin() + left_read_ptr, buf.begin() + right_write_ptr, buf.begin());
    left_read_ptr = 0;
    right_write_ptr = size;
}
void Buffer::buf_insert_tail(std::string_view str) {
    std::copy(str.begin(), str.end(), buf.begin() + right_write_ptr);
    right_write_ptr += str.length();
}

void Buffer::push(std::string_view str) {
    // todo 可以先写入socket中一部分
    auto n = str.length();
    auto remain_right = buf.capacity() - right_write_ptr;
    auto remain_left = left_read_ptr;

    if (n <= remain_right) {
        buf_insert_tail(str);
    } else if (n <= remain_left + remain_right) {
        buf_move_head();
        buf_insert_tail(str);
    } else {
        auto add = n - remain_left + remain_right;
        buf.resize(buf.capacity() + add);
        buf_move_head();
        buf_insert_tail(str);
    }
}


