#include <memory>
#include "TCPServer.hpp"
#include "TCPAcceptor.hpp"
#include "TCPConnection.hpp"
#include "EventLoop.hpp"

// todo 在类型中定义析构函数, 克服使用unique_ptr的时候报不完整type错误


TCPServer::TCPServer(const string& ip, int port, int n_thread):
    ip(ip),
    port(port),
    main_loop(new EventLoop()) // 需要一个main loop阻塞主线程
{
    if (n_thread != 0) {
        loop_pool = make_unique<EventloopPool>(n_thread);
    }
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

    // INFO(format("server_run | id: {}, ip: {}, port: {}", main_loop->thread_id, ip, port));
    main_loop->run();
}

void TCPServer::create_conn(int fd, const string peer_ip, const int peer_port) {
    auto cb = [this, fd, peer_ip, peer_port]() {
        assertm(conn_map.find(fd) == conn_map.end());

        if (conn_map.size() >= max_conn_num) {
            close(fd);
            return;
        }

        auto loop = loop_pool ? loop_pool->getLoop() : main_loop;
        auto cb = [this, fd] () {
            this->remove_conn(fd);
        };
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

        // INFO(format("create_conn | fd: {}, peer_id: {}, peer_port: {}", fd, peer_ip, peer_port));
    };

    main_loop->add_callback(cb);
}

void TCPServer::remove_conn(int fd) {
    auto cb = [this, fd] {
        auto iter = this->conn_map.find(fd);
        this->conn_map.erase(iter); 

        // INFO(format("remove_conn | fd: {}", fd));
    };

    main_loop->add_callback(cb);
}

