#pragma once
#include "base.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include "Connector.h"


class TcpClient: public noncopyable {
public:
    TcpClient(const std::string& host, port_t port);

    void run();

    UserOpenCallback openCallback;
    UserCloseCallback closeCallback;
    UserMessageCallback messageCallback;
private:
    void create_conn(fd_t fd, const string peer_ip, const int peer_port);
    void remove_conn();

    Eventloop* main_loop;
    const string host;
    const string ip;
    port_t port;

    std::unique_ptr<Connector> connector;
    std::unique_ptr<TcpConnection> conn;
};

