#pragma once
#include "base.h"

 // 实际上是一个可以扩容的队列, 右边插入, 左边弹出. 当插入容量不够时, 左边有空余移到左边, 否则resize
class Buffer: public noncopyable {
public:
    Buffer();
    ~Buffer() = default;

    uint32_t pushFrom(fd_t fd);
    void push(const std::string_view str);
    uint32_t popTo(fd_t fd);
    void pop();
    std::string_view peek();
    uint32_t getSize();

private:
    char* begin();
    void buf_move_head();
    void buf_insert_tail(const std::string_view str);

    uint32_t left_read_ptr;
    uint32_t right_write_ptr;
    std::vector<char> buf;
    static const uint32_t init_capa = 1024;
};


