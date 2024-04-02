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

    // todo 为什么this指针可以调用private成员函数
    if (open_cb) {
        auto cb = [this] {
            this->open_cb(this);
        };
        loop->addCallback(cb); 
    }
}

void TcpConnection::shutdown() {
    assertm(::shutdown(fd, SHUT_WR));

    // 我方关闭写端
    change_state(TcpConnectionState::WRITE_CLOSE);
}

void TcpConnection::send(const std::string& msg) {
    assertm(state == TcpConnectionState::CONNECTING);

    write_buf->push(msg);
    enableEvent(EventType::WRITE);
}

void TcpConnection::change_state(TcpConnectionState next_state) {
    if (state == TcpConnectionState::READ_CLOSE && next_state == TcpConnectionState::WRITE_CLOSE) {
        state = TcpConnectionState::CLOSE;
    } else if (state == TcpConnectionState::WRITE_CLOSE && next_state == TcpConnectionState::READ_CLOSE) {
        state = TcpConnectionState::CLOSE;
    } else if (state == next_state) {
        throw;
    } else {
        state = next_state;
    }

    if (next_state == TcpConnectionState::CLOSE) {
        disableEvent(EventType::READ);
        disableEvent(EventType::WRITE);

        if (close_cb) close_cb(this);

        // 如何conn与server在不同的loop，避免自己运行的时候删除自己
        loop->addCallbackAfter(remove_conn_cb);
    } else if (next_state == TcpConnectionState::READ_CLOSE) {
        disableEvent(EventType::READ);
    } else if (next_state == TcpConnectionState::WRITE_CLOSE) {
        disableEvent(EventType::WRITE);
    } else if (next_state == TcpConnectionState::CONNECTING) {
        throw;
    } else {
        throw;
    }
}

void TcpConnection::handle_read() {
    assertm(loop->isInSameThread());

    auto n = read_buf->pushFrom(fd);
    if (n == 0) {
        // 对方关闭连接 
        change_state(TcpConnectionState::CLOSE);
    } else if (n < 0) {
        switch(errno) {
            case EAGAIN:
            case EINTR:
                return;
            default:
                // 对方关闭读端
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

    if (write_buf->getSize() == 0) {
        disableEvent(EventType::WRITE);
    }
}
