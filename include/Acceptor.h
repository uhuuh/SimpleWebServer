#pragma once
#include "type.h"

class Acceptor: public noncopyable {
public:
    Acceptor(Eventloop* loop, NewConnectionCallback newConnectionCallbackconst, const std::string& host, in_port_t port);
    ~Acceptor();
    InetAddress getAddress() const {return m_addr;}

    void _handleRead();
    fd_t _createListenFd();
    
    Eventloop* m_loop;
    NewConnectionCallback m_newConnctionCallback;
    InetAddress m_addr;
    fd_t m_fd;
    std::unique_ptr<Channel> m_channel;
    inline const static uint32_t m_maxQueueSize = SOMAXCONN;
};