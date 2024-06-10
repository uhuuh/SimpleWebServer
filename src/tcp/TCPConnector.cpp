#include <arpa/inet.h>
#include <cassert>
#include <fcntl.h>
#include "TCPConnector.hpp"
#include "EventLoop.hpp"
#include "base.hpp"


void set_fd_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    assert(flags >= 0);

    // 添加 O_NONBLOCK 标志
    flags |= O_NONBLOCK;
    assert(fcntl(fd, F_SETFL, flags) >= 0);
}

TCPConnector::TCPConnector(
    EventLoop* loop, 
    CreateConnectionCallback create_conn_cb,
    const std::string& ip,
    int port
):
    loop(loop),
    fd(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)),
    ch(loop->get_channel(fd)),
    create_conn_cb(create_conn_cb),
    ip(ip),
    port(port)
{
    auto cb = bind(&TCPConnector::connect_peer, this);
    loop->add_callback(cb);
}


void TCPConnector::connect_peer() {
    assertm(loop->is_same_thread());

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    auto ret = connect(fd,  (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        if (retry_delay_ms <= max_retry_delay_ms) {
            // INFO(format("connect_retry | after_ms: {}", retry_delay_ms));
            auto cb = bind(&TCPConnector::connect_peer, this);
            loop->add_timer(cb, retry_delay_ms);
            retry_delay_ms *= 2;
        } else {
            throw;
        }
    } else {
        // INFO(format("connect_succ | after_ms: {}", retry_delay_ms));
        // 以阻塞方式建立连接，然后将fd设为非阻塞形式，然后使用loop监听该fd
        set_fd_nonblock(fd);
        create_conn_cb(fd, ip, port);
    }
}

