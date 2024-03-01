#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"

bool socketSelfConnect(fd_t fd) {
    auto localaddr = Socket::getLocalAddress(fd);
    auto ptr1 = reinterpret_cast<sockaddr_in*>(&localaddr);
    auto peeraddr = Socket::getPeerAddress(fd);
    auto ptr2 = reinterpret_cast<sockaddr_in*>(&peeraddr); 
    return ptr1->sin_port == ptr2->sin_port && ptr1->sin_addr.s_addr == ptr2->sin_addr.s_addr;
}

int socketGetError(fd_t fd) {
    int optval;
    socklen_t optlen = sizeof optval;
    int err = ::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);
    if (err < 0) err = errno;
    else err = optval;
    return err;
}

Connector::Connector(
    Eventloop* loop, 
    NewConnectionCallback newConnectionCallback,
    const std::string& host,
    in_port_t port
):
    m_loop(loop),
    m_newConnectionCallback(newConnectionCallback),
    m_peerAddr(host, port)
{
    _connect();
    LOG_TRACE << "init " << m_loop->m_threadId;
}


void Connector::_connect() {
    LOG_TRACE << "connecting part1";
    m_fd = ::socket( AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    assertm(m_fd >= 0);

    auto addr = reinterpret_cast<sockaddr*>(&m_peerAddr);
    auto err = ::connect(m_fd, addr, sizeof(sockaddr_in));
    if (err != 0) err = errno;
    switch (err)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            // assertm(!m_channel); // 之前调用removeChannel并没有清除unique_ptr内部的channel
            m_channel.reset(new Channel(m_fd));
            m_channel->addEvent(EventType::WRITE, [this]{_handleWrite();});
            m_channel->addEvent(EventType::EXCEPTION, [this]{_handleException();});
            m_loop->updateChannel(m_channel.get(), false);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            _retry();
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            FATAL("connector connect error");
            ::close(m_fd);
            break;

        default:
            FATAL("connector other error");
            ::close(m_fd);
            break;
    }
}

void Connector::_retry() {
    m_retryDelayMs = std::min(m_retryDelayMs * 2, m_maxRetryDelayMs);
    LOG_TRACE << "retry " << m_retryDelayMs;
    m_loop->addTimerAfter([this]{_connect();}, m_retryDelayMs, 0);
}

void Connector::_handleWrite() {
    LOG_TRACE << "connecting part2";
    m_loop->removeChannel(m_fd, false);

    if (socketGetError(m_fd)) {
        _retry();
    } else if (socketSelfConnect(m_fd)) {
        _retry();
    } else {
        LOG_TRACE << "connected";
        m_newConnectionCallback(m_fd);
    }
}

void Connector::_handleException() {
    LOG_TRACE << "handleException";
    m_loop->removeChannel(m_fd, false);
    _retry();
}


