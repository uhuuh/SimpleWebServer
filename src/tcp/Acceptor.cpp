#include "Acceptor.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>


fd_t createListenFd(const string& ip, in_port_t port) {
    auto fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    assertm(fd >= 0);

    int reuse = 1;
    assertm(::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) >= 0); // 设置复用

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    // address.sin_addr.s_addr = INADDR_ANY; // 监听所有网络接口
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    addr.sin_port = htons(port);

    assertm(bind(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) >= 0);
    assertm(listen(fd, -1) >= 0); // todo listen 队列最大值

    INFO(format("accept_init_listen | fd: {}", fd));
    return fd;
}

Acceptor::Acceptor(Eventloop* loop, CreateConnectionCallback create_conn_cb, const std::string& ip, in_port_t port):
    Channel(loop, createListenFd(ip, port)),
    create_conn_cb(create_conn_cb),
    ip(ip),
    port(port)
{
    addEvent(EventType::READ, [this](){this->handle_read();}); // note 函数绑定必须放构造函数的最后一行，但是书上说不行
}

void Acceptor::handle_read() {
    assertm(loop->isInSameThread());

    struct sockaddr_in addr;
    socklen_t len;
    auto peer_fd = ::accept(this->fd, reinterpret_cast<sockaddr*>(&addr), &len);
    assertm(peer_fd >= 0);

    char peer_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), peer_ip, INET_ADDRSTRLEN);
    int peer_port = ntohs(addr.sin_port);

    // todo ip和port都是0
    INFO(format("accept | peer_fd: {}, peer_ip: {}, peer_port: {}", peer_fd, peer_ip, peer_port));

    // peer_fd 是自愿，如果这里直接终止，fd不会被connection保护，不会被调用close
    create_conn_cb(peer_fd, peer_ip, peer_port);
}


