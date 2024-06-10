#pragma once
#include "EventLoop.hpp"
#include "base.hpp"
#include <memory>


class TCPAcceptor {
public:
    TCPAcceptor(EventLoop* loop, CreateConnectionCallback create_conn_cb, const std::string& host, int port);
private:
    void handle_read();

    EventLoop* loop;
    int fd;
    unique_ptr<EventLoop::Channel> ch;
    CreateConnectionCallback create_conn_cb;
    const string ip;
    const int port;
};