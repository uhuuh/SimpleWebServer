#include "TCPConnection.hpp"
#include "EventLoop.hpp"
#include "Buffer.hpp"
#include "base.hpp"
#include <memory>
#include <string_view>
#include <unistd.h>
#include <sys/socket.h>


TCPConnection::TCPConnection(
    EventLoop* loop,
    int fd, 
    const string& local_ip,
    const int local_port,
    const string& peer_ip,
    const int peer_port,
    UserCallback& user_cb,
    RemoveConnectionCallback remove_conn_cb
):
    loop(loop),
    fd(fd),
    ch(loop->get_channel(fd)),
    local_ip(local_ip),
    local_port(local_port),
    peer_ip(peer_ip),
    peer_port(peer_port),
    user_cb(user_cb),
    remove_conn_cb(remove_conn_cb),
    state(State::CONNECTING)
{
    write_buf = make_unique<Buffer>();
    read_buf = make_unique<Buffer>();

    auto read_cb = bind(&TCPConnection::handle_read, this);
    auto write_cb = bind(&TCPConnection::handle_write, this);
    ch->set_event(EventLoop::EventType::READ, read_cb);
    ch->set_event(EventLoop::EventType::WRITE, write_cb);
    ch->disable_event(EventLoop::EventType::WRITE);

    if (user_cb.open_cb) {
        auto cb = [this] {
            this->user_cb.open_cb(this);
        };
        loop->add_callback(cb); 
    }
}

void TCPConnection::half_close() {
    // 我方关闭写端
    // INFO(format("conn_write_close: fd: {}", fd));
    // change_state(TcpConnectionState::WRITE_CLOSE);

    state = State::HALF_CLOSE;
    if (write_buf->get_size() == 0 && !ch->is_enable(EventLoop::EventType::WRITE)) {
        shutdown(fd, SHUT_WR);
    }
}

void TCPConnection::full_close() {
    state = State::FULL_CLOSE;
    shutdown(fd, SHUT_RDWR);
    remove_conn_cb(); // note 这里销毁conn，后面没有使用对象成员，应该不会有什么问题
}

void TCPConnection::send_data(const std::string& msg) {
    assertm(state == State::CONNECTING);

    if (write_buf->get_size() + msg.size() >= max_write_buf_size) {
        LOG_ERROR("write buf too big");
        full_close();
        return;
    }

    write_buf->push(msg);
    ch->enable_event(EventLoop::EventType::WRITE);
}

string_view TCPConnection::get_data() {
    return read_buf->peek();
}

void TCPConnection::pop_data(int len) {
    read_buf->pop(len);
}

void TCPConnection::handle_read() {
    assertm(loop->is_same_thread());

    // note epoll有检测异常事件，但是不能直接知道是什么异常，通过read或者write返回-1，再根据errno可以知道是什么异常
    auto n = read_buf->push_from(fd);
    if (n > 0) {
        if (user_cb.message_cb) {
            user_cb.message_cb(this);
        } else {
            // 没有message_cb处理buf中的数据，这里pop，防止堆积
            read_buf->pop();
        }
    } else if (n < 0) {
        switch(errno) {
            case EAGAIN:
            case EINTR:
                // 可忽视的错误
                return;
                break;
            default:
                // 其他错误
                full_close();
        }
    } else {
        // 对方关闭了连接
        full_close();
    }
}

void TCPConnection::handle_write() {
    assertm(loop->is_same_thread());
    assertm(write_buf->get_size() > 0);
    // todo logger 如何打印出堆栈

    auto n = write_buf->pop_to(fd);
    // todo 如果write返回错误，EWOULDBLOCK，EPIPE，ECONNRESET
    // todo write_buf 剩余太多，调用高水位回调
    if (n > 0) {
        if (write_buf->get_size() == 0) {
            ch->disable_event(EventLoop::EventType::WRITE);

            if (state == State::HALF_CLOSE) {
                shutdown(fd, SHUT_WR);
            }
        }
    } else if (n < 0) {
        switch(errno) {
            case EAGAIN:
            case EINTR:
                // 可忽视的错误
                return;
                break;
            default:
                // 其他错误
                full_close();
        }
    } else {
        // 这个分支应该不可能
    }

}

