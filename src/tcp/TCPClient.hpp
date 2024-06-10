#pragma once
#include "base.hpp"

class EventLoop;
class TCPConnector;

class TCPClient: public noncopyable {
public:
    TCPClient(const std::string& host, int port);

    void run();

    UserCallback user_cb;
private:
    void create_conn(int fd, const string peer_ip, const int peer_port);
    void remove_conn();

    EventLoop* main_loop;

    const string host;
    const string ip;
    int port;

    std::unique_ptr<TCPConnector> connector;
    std::unique_ptr<TCPConnection> conn;
};

