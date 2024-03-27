#pragma once
#include "base.h"

enum class TcpConnectionState {
    CONNECTING,
    READ_CLOSE,
    WRITE_CLOSE,
    DISCONNECT,
    CLOSE
};

// todo 实现应用层保活机制

class TcpConnection: noncopyable {
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

    ~TcpConnection();

    void send(const std::string msg);
    void shutdown();

    void beforeDestroy(std::shared_ptr<TcpConnection> conn);
    void handle_read();
    void handle_write();

    void _afterInit();

    UserOpenCallback open_cb;
    UserCloseCallback close_cb;
    UserMessageCallback message_cb;
    RemoveConnectionCallback remove_conn_cb;

    fd_t fd;
    const string& local_ip;
    const int local_port;
    const string& peer_ip;
    const int peer_port;

    Eventloop* loop;
    std::unique_ptr<Channel> ch;
    std::unique_ptr<Buffer> in_buf;
    std::unique_ptr<Buffer> out_buf;

    TcpConnectionState state;
};