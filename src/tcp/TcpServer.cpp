#include "TcpServer.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "ThreadEventloop.h"
#include "ThreadPoolEventloop.h"
// todo 在类型中定义析构函数, 克服使用unique_ptr的时候报不完整type错误
// todo 替换成pimpl模式


TcpServer::TcpServer(const char* host, in_port_t port, int n_thread):
    m_loop(std::make_unique<Eventloop>()),
    m_loops(std::make_unique<ThreadPoolEventloop>(m_loop.get(), n_thread)),
    m_acceptor(std::make_unique<Acceptor>(
        m_loop.get(),
        [this](fd_t fd){_createConnection(fd);},
        host, 
        port
    )),
    m_connectionMap()
{
}

void TcpServer::run() {
    LOG_TRACE << "run";
    m_loop->loop();
}

void TcpServer::_createConnection(fd_t fd) {
    m_loop->assertSameThread();
    assertm(m_connectionMap.find(fd) == m_connectionMap.end());

    auto loop = m_loops->getLoop();
    auto ptr = std::make_shared<TcpConnection>(
        loop,
        [this](fd_t fd){_removeConnection(fd);},
        m_openCallback,
        m_closeCallback,
        m_messageCallback,
        fd,
        m_acceptor->getAddress(),
        Socket::getPeerAddress(fd)
    );
    m_connectionMap[fd] = ptr;
    LOG_TRACE << "create conn, fd=" << fd << ", addr=" <<  ptr->m_peerAddr.toString();
}

void TcpServer::_removeConnection(fd_t fd) {
    auto fun = [this, fd] {
        auto iter = m_connectionMap.find(fd);
        assertm(iter != m_connectionMap.end());
        auto ptr = iter->second;
        m_connectionMap.erase(iter);
        assertm(ptr.use_count() == 1);

        LOG_TRACE << "remove conn , fd=" << fd << ", addr=" << ptr->m_peerAddr.toString();
        ptr->beforeDestroy(ptr);
    };
    m_loop->run(fun, false);
}


