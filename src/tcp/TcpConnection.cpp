#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "Logger.h"
#include "base.h"
#include <memory>
#include <unistd.h>
#include <sys/socket.h>


TcpConnection::TcpConnection(
    Eventloop* loop,
    fd_t fd, 
    const string& local_ip,
    const int local_port,
    const string& peer_ip,
    const int peer_port,
    UserOpenCallback open_cb,
    UserCloseCallback close_cb,
    UserMessageCallback message_cb,
    RemoveConnectionCallback remove_conn_cb
):
    Channel(loop, fd),
    local_ip(local_ip),
    local_port(local_port),
    peer_ip(peer_ip),
    peer_port(peer_port),
    open_cb(open_cb),
    close_cb(close_cb),
    message_cb(message_cb),
    remove_conn_cb(remove_conn_cb),
    state(TcpConnectionState::CONNECTING)
{
    write_buf = make_unique<Buffer>();
    read_buf = make_unique<Buffer>();

    addEvent(EventType::READ, [this]{handle_read();});
    addEvent(EventType::WRITE, [this]{handle_write();});
    disableEvent(EventType::WRITE);

    if (open_cb) {
        auto cb = [this] {
            this->open_cb(this);
        };
        loop->addCallback(cb); 
    }
}

void TcpConnection::shutdown() {
    // 我方关闭写端
    INFO(format("conn_write_close: fd: {}", fd));
    change_state(TcpConnectionState::WRITE_CLOSE);
}

void TcpConnection::send(const std::string& msg) {
    // todo 状态管理，什么状态下可以调用什么样的成员函数。send，shutdown，forceClose
    assertm(state == TcpConnectionState::CONNECTING);

    write_buf->push(msg);
    enableEvent(EventType::WRITE);
}

void TcpConnection::change_state(TcpConnectionState next_state) {
    if (state == TcpConnectionState::READ_CLOSE && next_state == TcpConnectionState::WRITE_CLOSE) {
        next_state = TcpConnectionState::CLOSE;
    } else if (state == TcpConnectionState::WRITE_CLOSE && next_state == TcpConnectionState::READ_CLOSE) {
        next_state = TcpConnectionState::CLOSE;
    }
    state = next_state;


    if (state == TcpConnectionState::READ_CLOSE) {
        disableEvent(EventType::READ);
    } else {
        shutdown_conn_if();
    }
}

void TcpConnection::handle_read() {
    assertm(loop->isInSameThread());

    // note epoll有检测异常事件，但是不能直接知道是什么异常，通过read或者write返回-1，再根据errno可以知道是什么异常
    auto n = read_buf->pushFrom(fd);
    if (n == 0) {
        // 对方关闭连接 
        INFO(format("conn_close | fd: {}", fd));
        change_state(TcpConnectionState::CLOSE);
    } else if (n < 0) {
        switch(errno) {
            // todo

            case EAGAIN:
            case EINTR:
                return;
            default:
                // 对方关闭读端
                INFO(format("conn_read_close: fd: {}", fd));
                change_state(TcpConnectionState::READ_CLOSE);
        }
    } else {
        if (message_cb) {
            message_cb(this, read_buf.get());
        } else {
            // 没有message_cb处理buf中的数据，这里pop，防止堆积
            read_buf->pop();
        }
    }
}

void TcpConnection::handle_write() {
    assertm(loop->isInSameThread());

    // todo logger 如何打印出堆栈
    assertm(write_buf->getSize() > 0);

    write_buf->popTo(fd);
    // todo 如果write返回错误，EWOULDBLOCK，EPIPE，ECONNRESET
    // todo write_buf 剩余太多，调用高水位回调


    if (write_buf->getSize() == 0) {
        disableEvent(EventType::WRITE); 

        shutdown_conn_if();
    }
}

void TcpConnection::shutdown_conn_if() {
    if (write_buf->getSize() != 0) return;

    if (state == TcpConnectionState::CLOSE) {
        disableEvent(EventType::WRITE);
        disableEvent(EventType::READ);
        // note 由于我方只会关闭写端，故执行到此处连接已经关闭，无需再执行下面函数关闭连接
        // assertm(::shutdown(fd, SHUT_RDWR) >= 0);
        // assertm(::shutdown(fd, SHUT_RD) >= 0);
        // assertm(::close(fd) >= 0);
 
        if (close_cb) close_cb(this);

        // note 防止在成员函数中执行delete this
        // muduo中conn是share_ptr，然后通过shared_from_this防止conn过早销毁
        loop->addCallbackAfter(remove_conn_cb);
    } else if (state == TcpConnectionState::WRITE_CLOSE) {
        disableEvent(EventType::WRITE);
        assertm(::shutdown(fd, SHUT_WR) >= 0);
    }
}