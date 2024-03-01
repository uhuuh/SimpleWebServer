#pragma once
#include "type.h"

class Buffer: public noncopyable {
public:
    Buffer(); // 实际上是一个可以扩容的队列, 右边插入, 左边弹出. 当插入容量不够时, 左边有空余移到左边, 否则resize
    ~Buffer() = default;

    uint32_t pushFrom(fd_t fd);
    void push(const std::string_view str);
    uint32_t popTo(fd_t fd);
    void pop();
    std::string_view peek();
    uint32_t getSize();
    char* _begin();
    void _bufferMoveHead();
    void _bufferInsertTail(const std::string_view str);

    uint32_t m_ptrLeftRead;
    uint32_t m_ptrRightWrite;
    std::vector<char> m_buffer;
    static const uint32_t m_initCapacity = 1024;
};


