#pragma once
#include "type.h"

class Activater: noncopyable {
public:
    Activater(Eventloop* loop);
    ~Activater();

    void activate();
    void _handleRead();

    fd_t m_fd;
    std::unique_ptr<Channel> m_channel;
    Eventloop* m_loop;
};