#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"
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
    loop(loop),
    fd(fd),
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
    ch->enableEvent(EventType::READ, [this]{handle_read();});

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

    if (state == TcpConnectionState::READ_CLOSE) {
        state = TcpConnectionState::CLOSE;
        if (close_cb) close_cb(this);
        ch->disableEvent(EventType::WRITE);
        remove_conn_cb(fd);
    } else {
        state = TcpConnectionState::WRITE_CLOSE;
        ch->disableEvent(EventType::WRITE);
    }
}


void TcpConnection::send(const std::string msg) {
    assertm(state == TcpConnectionState::CONNECTING);

    in_buf->push(msg);
    ch->enableEvent(EventType::WRITE, [this]{handle_write();});
}

TcpConnection::~TcpConnection() {
    ::close(fd);
}

void TcpConnection::handle_read() {
    auto n = out_buf->pushFrom(fd);
    if (n == 0) {

    } else if (n < 0) {
        switch(errno) {
            case EAGAIN:
            case EINTR:
                return;
            default:

                if (state == TcpConnectionState::WRITE_CLOSE) {
                    state = TcpConnectionState::CLOSE;
                    if (close_cb) close_cb(this);
                    ch->disableEvent(EventType::READ);
                    remove_conn_cb(fd);
                } else {
                    state = TcpConnectionState::READ_CLOSE;
                    ch->disableEvent(EventType::READ);
                }
        }
    } else {
        if (message_cb) {
            message_cb(this, out_buf.get());
        }
    }
}

void TcpConnection::handle_write() {
    // todo logger 如何打印出堆栈
    assertm(in_buf->getSize() > 0);

    in_buf->popTo(fd);

    if (in_buf->getSize() == 0) {
        ch->disableEvent(EventType::WRITE);
    }
}
