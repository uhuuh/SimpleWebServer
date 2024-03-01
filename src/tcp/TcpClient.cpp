#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "Channel.h"


TcpClient::TcpClient(const std::string& host, in_port_t port):
    m_loop(new Eventloop()),
    m_connector(new Connector(
        m_loop.get(), 
        [this](fd_t fd){_createConnection(fd);},
        host,
        port
    ))
{
}

void TcpClient::run() {
    LOG_TRACE << "run " << m_loop->m_threadId;
    m_loop->loop();
}

void TcpClient::_createConnection(fd_t fd) {
    m_loop->assertSameThread();
    assertm(!m_connection);

    auto loop = m_loop.get();
    m_connection = std::make_unique<TcpConnection>(
        loop,
        [this](fd_t fd){_removeConnection(fd);},
        m_openCallback,
        m_closeCallback,
        m_messageCallback,
        fd,
        Socket::getLocalAddress(fd),
        Socket::getPeerAddress(fd)
    );
    LOG_TRACE << "create conn, fd=" << fd << ", addr=" <<  m_connection->m_peerAddr.toString();
}

void TcpClient::_removeConnection(fd_t fd) {
    m_loop->assertSameThread();
    assertm(bool(m_connection));

    LOG_TRACE << "remove conn , fd=" << fd << ", addr=" << m_connection->m_peerAddr.toString();
    m_connection->beforeDestroy(std::move(m_connection));
    // todo 与服务器的连接意外中断, 可以重新调用connector的restart 重连
}