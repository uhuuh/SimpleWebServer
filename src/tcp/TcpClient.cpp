#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "EventloopPool.h"
#include "TcpConnection.h"
#include "Channel.h"
#include "base.h"
#include <memory>


TcpClient::TcpClient(const string& ip, port_t port):
    ip(ip),
    port(port),
    main_loop(new Eventloop())
{
}

void TcpClient::run() {
    // todo 优化下面的lambda，使用bind试试看
    connector = make_unique<Connector>(
        main_loop,
        [this](auto fd, auto ip, auto port){this->create_conn(fd, ip, port);},
        ip,
        port
    );

    INFO(format("client_run | id: {}, ip: {}, port: {}", main_loop->thread_id, ip, port));
    main_loop->run();
}

void TcpClient::create_conn(fd_t fd, const string& peer_ip, const int peer_port) {
    auto cb = [this, fd, peer_ip, peer_port]() {
        auto loop = main_loop;
        conn = std::make_unique<TcpConnection>(
            loop,
            fd, 
            ip,
            port,
            peer_ip,
            peer_port,
            openCallback,
            closeCallback,
            messageCallback,
            [this](auto fd){this->remove_conn(fd);}
        );
    };
    main_loop->addCallback(cb);
}

void TcpClient::remove_conn(fd_t fd) {
    // pass
}


