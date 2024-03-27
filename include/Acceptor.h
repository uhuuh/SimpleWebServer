#pragma once
#include "base.h"


class Acceptor: public noncopyable {
public:
    Acceptor(Eventloop* loop, CreateConnectionCallback create_conn_cb, const std::string& host, port_t port);
    ~Acceptor();
private:
    void handle_read();

    Eventloop* loop;
    CreateConnectionCallback create_conn_cb;
    const string ip;
    port_t port;
    fd_t fd;
    std::unique_ptr<Channel> ch;
};