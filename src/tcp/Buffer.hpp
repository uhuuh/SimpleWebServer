#pragma once
#include "base.hpp"


// 实际上是一个可以扩容的队列, 右边插入, 左边弹出. 当插入容量不够时, 左边有空余移到左边, 否则resize
class Buffer: public noncopyable {
public:
    Buffer();
    ~Buffer() = default;

    int push_from(int fd);
    void push(const std::string_view str);
    int pop_to(int fd);
    void pop(int len);
    void pop();
    std::string_view peek();
    int get_size();

private:
    char* begin();
    void buf_move_head();
    void buf_insert_tail(const std::string_view str);

    int left_read_ptr;
    int right_write_ptr;
    std::vector<char> buf;
    static const int init_capa = 1024;
};


