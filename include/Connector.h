#pragma once
#include "base.h"


class Connector {
public:
    Connector(Eventloop* loop, CreateConnectionCallback create_conn_cb, const std::string& ip, port_t port);

    void _handleException();
    void _connect();
    void _retry();

private:
    void connect();
    void handle_read();

    Eventloop *loop;
    const string ip;
    port_t port;
    CreateConnectionCallback create_conn_cb;

    fd_t fd;
    int retry_delay_ms = 500;
    static const int max_retry_delay_ms = 30 * 1000;
};