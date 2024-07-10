#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include "EventLoop.hpp"
#include "base.hpp"
#include "TCPAcceptor.hpp"



TCPAcceptor::TCPAcceptor(EventLoop* loop, CreateConnectionCallback create_conn_cb, const std::string& ip, int port):
    loop(loop),
    fd(createListenFd(ip, port)),
    ch(loop->get_channel(fd)),
    create_conn_cb(create_conn_cb),
    ip(ip),
    port(port)
{
    // 函数绑定必须放构造函数的最后一行
    auto cb = bind(&TCPAcceptor::handle_read, this);
    ch->set_event(EventLoop::EventType::READ, cb);
}

void TCPAcceptor::handle_read() {
    assertm(loop->is_same_thread());

    struct sockaddr_in addr;
    socklen_t len;
    auto peer_fd = ::accept(this->fd, reinterpret_cast<sockaddr*>(&addr), &len);
    assertm(peer_fd >= 0);
    // todo 处理fd耗尽问题

    char peer_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), peer_ip, INET_ADDRSTRLEN);
    int peer_port = ntohs(addr.sin_port);

    // todo ip和port都是0

    // peer_fd 是自愿，如果这里直接终止，fd不会被connection保护，不会被调用close
    create_conn_cb(peer_fd, peer_ip, peer_port);
}


