#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"


TcpConnection::TcpConnection(
    Eventloop* loop, 
    RemoveConnectionCallback remove, 
    UserOpenCallback openCb,
    UserCloseCallback closeCb,
    UserMessageCallback messageCb,
    fd_t fd, 
    InetAddress localAddr, 
    InetAddress peerAddr
):
    m_loop(loop),
    m_removeConnectionCallback(remove),
    m_openCallback(openCb),
    m_closeCallback(closeCb),
    m_messageCallback(messageCb),
    m_fd(fd),
    m_localAddr(localAddr),
    m_peerAddr(peerAddr),
    m_channel(new Channel(m_fd)),
    m_inputBuffer(new Buffer()),
    m_outputBuffer(new Buffer()),
    m_state(CONNECTING)
{
    
    m_channel->addEvent(EventType::READ, [this]{_handleRead();});
    m_channel->addEvent(EventType::WRITE, [this]{_handleWrite();});
    m_channel->addEvent(EventType::EXCEPTION, [this]{_handleException();});
    m_loop->updateChannel(m_channel.get(), false);
    LOG_TRACE << "conn add channel in loop";
    // note 修改channel的操作必须放在conn的loop中执行. 如果不是这样, channel添加进loop却没有activate, 导致loop要poll empty一次才能意识到loop上的修改

    auto fun = [this] {
        if (m_openCallback) {
            m_openCallback(this);
        }
    };
    m_loop->run(fun, false); // todo 为什么this指针可以调用private成员函数
    LOG_TRACE << "init " << m_loop->m_threadId;
}

void TcpConnection::shutdown() {
    assertm(m_state == CONNECTING);

    assertm(::shutdown(m_fd, SHUT_WR));
    m_state = DISCONNECTING;
}

void TcpConnection::beforeDestroy(std::shared_ptr<TcpConnection> conn) {
    // 不能再conn内部调用删除conn的函数, 由于在多线程环境下不能保证runafter一定会在conn内部执行完再调用删除conn的操作, 使用share_ptr保存引用使这个conn不被析构
    assertm(m_state == DISCONNECTED);

    auto fun = [this, conn]  {
        if (m_closeCallback) {// 用户回调必须在conn的loop中执行
            m_closeCallback(conn.get());
        }
    };
    m_loop->run(fun, true); 
    // note 这里必须是稍后执行, 因为现在只剩自己
}

void TcpConnection::send(const std::string msg) {
    assertm(m_state == CONNECTING);

    m_inputBuffer->push(msg);

    LOG_TRACE << "-> " << m_fd;
}

TcpConnection::~TcpConnection() {
    ::close(m_fd);
    LOG_TRACE << "destroy " << m_loop->m_threadId;
}

void TcpConnection::_handleRead() {
    m_loop->assertSameThread();
    LOG_TRACE << "<- " << m_fd;

    auto n = m_outputBuffer->pushFrom(m_fd);
    if (n <= 0) {
        if (n < 0) LOG_TRACE << "conn error";
        m_state = DISCONNECTED;
        m_loop->removeChannel(m_fd, false);
        m_removeConnectionCallback(m_fd);
    } else {
        if (m_messageCallback) {
            m_messageCallback(this, m_outputBuffer.get());
        }
    }
}

void TcpConnection::_handleWrite() {
    m_loop->assertSameThread();

    // todo 只有buffer中有数据的时候, 开开启write事件
    if (m_inputBuffer->getSize() != 0) {
        m_inputBuffer->popTo(m_fd);
    }
}

void TcpConnection::_handleException() {
    m_loop->assertSameThread();
    assertm(-1);
}