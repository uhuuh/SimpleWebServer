#pragma once
#include "Channel.h"
#include "Buffer.h"
#include "base.h"
#include <memory>

enum class TcpConnectionState {
    CONNECTING,
    READ_CLOSE,
    WRITE_CLOSE,
    CLOSE
};

// todo 实现应用层保活机制

class TcpConnection: public Channel {
public:
    TcpConnection(
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
    );

    void send(const std::string& msg);
    void shutdown();

    TcpConnectionState state;
    const string local_ip;
    const int local_port;
    const string peer_ip;
    const int peer_port;
private:
    void handle_read();
    void handle_write();
    void change_state(TcpConnectionState);

    UserOpenCallback open_cb;
    UserCloseCallback close_cb;
    UserMessageCallback message_cb;
    RemoveConnectionCallback remove_conn_cb;

    std::unique_ptr<Buffer> write_buf;
    std::unique_ptr<Buffer> read_buf;
};