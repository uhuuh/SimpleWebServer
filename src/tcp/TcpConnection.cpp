#include "TCPConnection.hpp"
#include "EventLoop.hpp"
#include "Buffer.hpp"
#include "base.hpp"
#include "Logger.hpp"
#include <cassert>
#include <memory>
#include <string_view>
#include <unistd.h>
#include <sys/socket.h>
#include "Logger.hpp"


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
    peer_fd(fd),
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

    LOG_TRACE("conn init, %s", to_str().c_str());

    if (user_cb.open_cb) user_cb.open_cb(this);
}

void TCPConnection::half_close() {
    assert(state == State::CONNECTING);

    state = State::HALF_CLOSE;
    if (write_buf->get_size() == 0 && !ch->is_enable(EventLoop::EventType::WRITE)) {
        shutdown(peer_fd, SHUT_WR);
        LOG_TRACE("conn half_close, %s", to_str().c_str());
    }
}

void TCPConnection::full_close() {
    assertm(state != State::FULL_CLOSE);

    state = State::FULL_CLOSE;
    shutdown(peer_fd, SHUT_RDWR);
    LOG_TRACE("conn full_close, %s", to_str().c_str());

    if (user_cb.close_cb) user_cb.close_cb(this);

    loop->add_callback_after(remove_conn_cb); // note add_callback_after添加的回调一定不会立刻执行
}

string TCPConnection::to_str() {
    return to_format_str("fd=%d ip=%s port=%d state=%d", peer_fd, peer_ip.c_str(), peer_port, (int)state);
}

void TCPConnection::send_data(const std::string& msg) {
    assertm(state == State::CONNECTING);

    if (write_buf->get_size() + msg.size() >= max_write_buf_size) {
        LOG_ERROR("conn wirte_buf too big");
        full_close();
        return;
    }

    write_buf->push(msg);
    ch->enable_event(EventLoop::EventType::WRITE);
}

string_view TCPConnection::get_data() {
    assertm(state != State::FULL_CLOSE);

    return read_buf->peek();
}

void TCPConnection::pop_data(int len) {
    assertm(state != State::FULL_CLOSE);

    read_buf->pop(len);
}

void TCPConnection::pop_data() {
    assertm(state != State::FULL_CLOSE);

    read_buf->pop(); 
}

void TCPConnection::handle_read() {
    assertm(loop->is_same_thread());

    auto n = read_buf->push_from(peer_fd);
    if (n > 0) {
        LOG_TRACE("conn read %d byte, %s", n, to_str().c_str());

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

    auto n = write_buf->pop_to(peer_fd);
    if (n > 0) {
        LOG_TRACE("conn write %d byte, %s", n, to_str().c_str());

        if (write_buf->get_size() == 0) {
            ch->disable_event(EventLoop::EventType::WRITE);

            if (user_cb.write_lower) user_cb.write_lower(this);

            if (state == State::HALF_CLOSE) {
                shutdown(peer_fd, SHUT_WR);

                LOG_TRACE("conn half_close, %s", to_str().c_str());
            }
        } else if (write_buf->get_size() >= max_write_buf_size) {
            if (user_cb.write_high) user_cb.write_high(this);
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
        assertm(-1);
    }

}

