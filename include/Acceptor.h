#pragma once
#include "Channel.h"
#include "base.h"

class Acceptor: public Channel {
public:
    Acceptor(Eventloop* loop, CreateConnectionCallback create_conn_cb, const std::string& host, port_t port);
private:
    void handle_read();

    CreateConnectionCallback create_conn_cb;
    const string ip;
    port_t port;
};