#pragma once
#include "EventLoop.hpp"
#include <memory>
#include "tcp_fun.hpp"


class EventLoop;
class TCPConnector {
public:
    TCPConnector(EventLoop* loop, CreateConnectionCallback create_conn_cb, const std::string& ip, int port);
private:
    void connect_peer();
    void handle_read();

    int fd;
    EventLoop *loop;
    unique_ptr<EventLoop::Channel> ch;

    const string ip;
    const int port;
    CreateConnectionCallback create_conn_cb;

    int retry_delay_ms = 500;
    static const int max_retry_delay_ms = 30 * 1000;
};