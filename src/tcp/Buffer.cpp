#include "Buffer.h"


uint32_t Buffer::pushFrom(fd_t fd) {
    auto remainRight = m_buffer.capacity() - m_ptrRightWrite;
    // todo 通过readv结果, 如果结果在栈空间上则自动扩容
    if (remainRight < m_initCapacity / 2) {
        m_buffer.resize(m_buffer.capacity() * 2);
        remainRight = m_buffer.capacity() - m_ptrRightWrite;
    }
    auto n = ::read(fd, _begin() + m_ptrRightWrite, remainRight);
    if (n >= 0) {
        m_ptrRightWrite += n;
        return static_cast<uint32_t>(n);
    } else {
        return n;
    }
}

uint32_t  Buffer::popTo(fd_t fd) {
    auto n = ::write(fd, _begin() + m_ptrLeftRead, getSize());
    assertm(n >= 0);
    m_ptrLeftRead += n;
    return static_cast<uint32_t>(n);
}


uint32_t Buffer::getSize() {
    return m_ptrRightWrite - m_ptrLeftRead;
}

Buffer::Buffer():
        m_ptrLeftRead(0),
        m_ptrRightWrite(0),
        m_buffer(m_initCapacity)
{}

char *Buffer::_begin() {
    return &(*m_buffer.begin());
}

std::string_view Buffer::peek() {
    return {_begin() + m_ptrLeftRead, getSize()};
}

void Buffer::pop() { // 弹出左边的
    auto n = peek().size();
    m_ptrLeftRead += n;
}

void Buffer::_bufferMoveHead() {
    auto size = getSize();
    std::copy(m_buffer.begin() + m_ptrLeftRead, m_buffer.begin() + m_ptrRightWrite, m_buffer.begin());
    m_ptrLeftRead = 0;
    m_ptrRightWrite = size;
}
void Buffer::_bufferInsertTail(std::string_view str) {
    std::copy(str.begin(), str.end(), m_buffer.begin() + m_ptrRightWrite);
    m_ptrRightWrite += str.length();
}

void Buffer::push(std::string_view str) {
    // 可以先写入socket中一部分
    auto n = str.length();
    auto remainRight = m_buffer.capacity() - m_ptrRightWrite;
    auto remainLeft = m_ptrLeftRead;

    if (n <= remainRight) {
        _bufferInsertTail(str);
    } else if (n <= remainLeft + remainRight) {
        _bufferMoveHead();
        _bufferInsertTail(str);
    } else {
        auto add = remainLeft + remainRight - n;
        m_buffer.resize(m_buffer.capacity() + add);
        _bufferMoveHead();
        _bufferInsertTail(str);
    }
}


