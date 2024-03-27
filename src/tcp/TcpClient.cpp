#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "EventloopPool.h"
#include "TcpConnection.h"
#include "Channel.h"


TcpClient::TcpClient(const string& ip, port_t port):
    ip(ip),
    port(port),
    loop_pool(new EventloopPool(1)),
    loop(loop_pool->getLoop())
{
}

void TcpClient::run() {
    LOG_TRACE << "run";
    // todo 优化下面的lambda，使用bind试试看
    connector.reset(new Connector(
        loop,
        [this](auto fd, auto ip, auto port){this->create_conn(fd, ip, port);},
        ip,
        port
    ));
}

void TcpClient::create_conn(fd_t fd, const string& peer_ip, const int peer_port) {
    auto cb = [this, fd, peer_ip, peer_port]() {
        auto loop = loop_pool->getLoop();
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
    loop->addCallback(cb);
}

void TcpClient::remove_conn(fd_t fd) {
    // pass
}


