#include <memory>
#include <string>
#include "TCPServer.hpp"
#include "EventloopPool.hpp"
#include "Logger.hpp"
#include "TCPAcceptor.hpp"
#include "TCPConnection.hpp"
#include "EventLoop.hpp"
#include "base.hpp"

// todo 在类型中定义析构函数, 克服使用unique_ptr的时候报不完整type错误


TCPServer::TCPServer(const string& ip, int port, int n_thread):
    ip(ip),
    port(port)
{
    assertm(n_thread >= 1);
    loop_pool = make_unique<EventloopPool>(n_thread);
    main_loop = loop_pool->getLoop();
}

void TCPServer::run() {
    auto cb = [this] (auto fd, auto peer_ip, auto peer_port) {
        this->create_conn(fd, peer_ip, peer_port);
    };
    acceptor = make_unique<TCPAcceptor>(
        main_loop, 
        cb, 
        ip, 
        port
    );

    loop_pool->start();
    loop_pool->wait_stop();
}

void TCPServer::stop() {
    loop_pool->stop();
}

void TCPServer::create_conn(int fd, const string peer_ip, const int peer_port) {
    auto cb = [this, fd, peer_ip, peer_port]() {
        // string conn_name = peer_ip + "-" + to_string(peer_port) + "-" + to_string(fd);
        assertm(conn_map.find(fd) == conn_map.end());

        if (conn_map.size() >= max_conn_num) {
            close(fd);
            LOG_ERROR("server close_conn for too many conn, fd=%d ip=%s port=%d", fd, peer_ip.c_str(), peer_port);
            return;
        }

        // LOG_TRACE("server create_conn, fd=%d ip=%s port=%d", fd, peer_ip.c_str(), peer_port);

        auto loop = loop_pool ? loop_pool->getLoop() : main_loop;
        auto cb = bind(&TCPServer::remove_conn, this, fd);
        conn_map[fd] = std::make_unique<TCPConnection>(
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

void TCPServer::remove_conn(int fd) {
    auto cb = [this, fd] {
        auto iter = this->conn_map.find(fd);
        string conn_str = iter->second->to_str();
        this->conn_map.erase(iter); 
        // LOG_TRACE("server destroy conn, %s", conn_str.c_str());
    };

    main_loop->add_callback(cb);
}

