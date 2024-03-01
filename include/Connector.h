#pragma once
#include "type.h"

class Connector {
public:
    Connector(Eventloop* loop, NewConnectionCallback newConnectionCallback, const std::string& host, in_port_t port);

    void _handleWrite();
    void _handleException();
    void _connect();
    void _retry();

    Eventloop *m_loop;
    NewConnectionCallback m_newConnectionCallback;
    InetAddress m_localAddr;
    InetAddress m_peerAddr;
    fd_t m_fd;
    std::unique_ptr<Channel> m_channel;
    int m_retryDelayMs = 500;
    inline static const int m_maxRetryDelayMs = 30 * 1000;
};