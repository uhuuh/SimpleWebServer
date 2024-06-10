#pragma once
#include "base.hpp"
#include "TCPConnector.hpp"
#include "TCPConnection.hpp"
#include <memory>
using namespace std;

// todo unique ptr 完整类型

class TCPClient: public noncopyable {
public:
    TCPClient(const std::string& host, int port);

    void run();
    void stop();

    UserCallback user_cb;
private:
    void create_conn(int fd, const string peer_ip, const int peer_port);
    void remove_conn();

    EventLoop* main_loop;

    const string host;
    const string ip;
    int port;

    unique_ptr<TCPConnector> connector;
    unique_ptr<TCPConnection> conn;
};

