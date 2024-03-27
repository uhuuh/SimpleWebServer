#include "TcpServer.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "EventloopPool.h"
#include "EventLoop.h"
#include <memory>
// todo 在类型中定义析构函数, 克服使用unique_ptr的时候报不完整type错误
// todo 替换成pimpl模式


TcpServer:: TcpServer(const string& ip, port_t port, int n_thread):
    ip(ip),
    port(port),
    loop_pool(new EventloopPool(n_thread)),
    loop(loop_pool->getLoop())
{
}

void TcpServer::run() {
    LOG_TRACE << "run";
    acceptor.reset(new Acceptor(
        loop_pool->getLoop(), 
        [this](auto fd, auto peer_ip, auto peer_port){this->create_conn(fd, peer_ip, peer_port);}, 
        ip, 
        port
    ));
}

void TcpServer::create_conn(fd_t fd, const string& peer_ip, const int peer_port) {
    auto cb = [this, fd, peer_ip, peer_port]() {
        assertm(conn_map.find(fd) == conn_map.end());

        auto loop = loop_pool->getLoop();
        conn_map[fd] = std::make_unique<TcpConnection>(
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

void TcpServer::remove_conn(fd_t fd) {
    auto cb = [this, fd] {
        remove_conn_set.push_back(fd);

        for (auto p = remove_conn_set.begin(); p != remove_conn_set.end(); ) {
            auto remove_fd = *p;
            if (conn_map[remove_fd]->state == TcpConnectionState::CLOSE) {
                conn_map.erase(remove_fd);
                p = remove_conn_set.erase(p);
            } else if (remove_fd == fd) {
                // 与tcpconnection同一个loop时，tcpconnection内调用remove_conn删除自己。这里跳过，等待下次删除
                ++p;
            } else {
                ++p;
            }
        }
    };
    loop->addCallback(cb);
}


