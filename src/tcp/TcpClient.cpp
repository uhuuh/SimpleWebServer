#include "EventLoop.hpp"
#include "EventLoopPool.hpp"
#include "TCPConnection.hpp"
#include "TcpClient.hpp"
#include "TCPConnector.hpp"
#include "base.hpp"
#include <memory>


TCPClient::TCPClient(const string& ip, int port):
    ip(ip),
    port(port),
    main_loop(new EventLoop())
{
}

void TCPClient::run() {
    // todo 优化下面的lambda，使用bind试试看
    auto cb = bind(&TCPClient::create_conn, this, placeholders::_1, placeholders::_2, placeholders::_3);
    connector = make_unique<TCPConnector>(main_loop, cb, ip, port);

    // INFO(format("client_run | id: {}, ip: {}, port: {}", main_loop->thread_id, ip, port));
    main_loop->run();
}

void TCPClient::create_conn(int fd, string peer_ip, int peer_port) {
    auto cb = [this, fd, peer_ip, peer_port]() {
        auto loop = main_loop;
        auto cb = bind(&TCPClient::remove_conn, this);

        conn = std::make_unique<TCPConnection>(
            loop,
            fd, 
            ip,
            port,
            peer_ip,
            peer_port,
            user_cb,
            cb
        );
    };
    main_loop->add_callback(cb);
}

void TCPClient::remove_conn() {
    // client销毁的同时也销毁conn
    main_loop->quit();
}


