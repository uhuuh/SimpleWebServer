#pragma once
#include <memory>
#include <unistd.h>
#include <unordered_map>
#include "base.hpp"
#include "tcp_fun.hpp"


class EventLoop;
class EventloopPool;
class TCPAcceptor;
class TCPServer: public noncopyable {
public:


    TCPServer(const string& ip, int port, int n_thread = 1);
    ~TCPServer() = default;

    void run();
    void stop();
    UserCallback user_cb;
private:
    void create_conn(int fd, const string peer_ip, const int peer_port);
    void remove_conn(int fd);

    const string ip;
    const int port;
    unique_ptr<EventloopPool> loop_pool;
    EventLoop* main_loop;
    unique_ptr<TCPAcceptor> acceptor;
    unordered_map<int, std::unique_ptr<TCPConnection>> conn_map;
    const static int max_conn_num = 20000;
};

