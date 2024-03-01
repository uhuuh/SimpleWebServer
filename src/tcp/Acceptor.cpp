#include "Acceptor.h"
#include "Channel.h"
#include "EventLoop.h"


Acceptor::Acceptor(
    Eventloop* loop, 
    NewConnectionCallback newConnectionCallback,
    const std::string& host,
    in_port_t port
):
    m_loop(loop),
    m_newConnctionCallback(newConnectionCallback),
    m_addr(host, port),
    m_fd(),
    m_channel()
{
    m_fd = _createListenFd();
    m_channel.reset(new Channel(m_fd));
    m_channel->addEvent(EventType::READ, [this]{_handleRead();});
    m_loop->updateChannel(m_channel.get(), false);
    LOG_TRACE << "listen fd=" << m_fd;
    LOG_TRACE << "init " << m_loop->m_threadId;
}

fd_t Acceptor::_createListenFd() {
    auto fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    assertm(fd >= 0);
    int reuse = 1;
    assertm(::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) >= 0); // 设置复用
    LOG_TRACE << "addr reuse";
    auto addr = reinterpret_cast<sockaddr *>(&m_addr);
    assertm(::bind(fd, addr, sizeof m_addr) >= 0);
    assertm(::listen(fd, m_maxQueueSize) >= 0);
    return fd;
}

void Acceptor::_handleRead() {
    m_loop->assertSameThread();

    InetAddress addr;
    socklen_t len;
    auto fd = ::accept(m_fd, reinterpret_cast<sockaddr*>(&addr), &len);
    assertm(fd >= 0);
    LOG_TRACE << "accept fd=" << fd;
    m_newConnctionCallback(fd);
}

Acceptor::~Acceptor() {
    close(m_fd);
    LOG_TRACE << "destroy " << m_loop->m_threadId;
}

