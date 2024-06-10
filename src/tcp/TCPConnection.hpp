#pragma once
#include "Buffer.hpp"
#include "base.hpp"
#include "EventLoop.hpp"
#include <memory>
#include <string_view>


// todo 实现应用层保活机制


class TCPConnection {
public:
    enum class State {
        CONNECTING,
        HALF_CLOSE,
        FULL_CLOSE
    };

    TCPConnection(
        EventLoop* loop, 
        int fd,
        const string& local_ip,
        const int local_port,
        const string& peer_ip,
        const int peer_port,
        UserCallback& user_cb,
        RemoveConnectionCallback remove_conn_cb
    );

    void send_data(const std::string& msg);
    string_view get_data();
    void pop_data();
    void pop_data(int len);
    void half_close();
    void full_close();
    string to_str();

    const string local_ip;
    const int local_port;
    const string peer_ip;
    const int peer_port;
    const int peer_fd;
private:
    void handle_read();
    void handle_write();
    void change_state(State);
    void shutdown_conn_if();

    State state;
    EventLoop* loop;
    unique_ptr<EventLoop::Channel> ch;

    UserCallback user_cb;
    RemoveConnectionCallback remove_conn_cb;

    std::unique_ptr<Buffer> write_buf;
    std::unique_ptr<Buffer> read_buf;
    const static int max_write_buf_size = 1024 * 1024;
};