#pragma once
#include "Channel.h"
#include "base.h"


class Connector: public Channel {
public:
    Connector(Eventloop* loop, CreateConnectionCallback create_conn_cb, const std::string& ip, port_t port);

    void _handleException();
    void _connect();
    void _retry();

private:
    void connect_peer();
    void handle_read();

    const string ip;
    const port_t port;
    CreateConnectionCallback create_conn_cb;

    int retry_delay_ms = 500;
    static const int max_retry_delay_ms = 30 * 1000;
};