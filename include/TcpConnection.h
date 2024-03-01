#pragma once
#include "type.h"

class TcpConnection: noncopyable {
public:
    TcpConnection(
        Eventloop* loop, 
        RemoveConnectionCallback remove, 
        UserOpenCallback openCb,
        UserCloseCallback closeCb,
        UserMessageCallback messageCb,
        fd_t fd, 
        InetAddress localAddr, 
        InetAddress peerAddr
    );
    ~TcpConnection();

    void send(const std::string msg);
    void shutdown();
    void beforeDestroy(std::shared_ptr<TcpConnection> conn);
    void _handleRead();
    void _handleWrite();
    void _handleException();
    void _afterInit();

    enum state {CONNECTING, DISCONNECTING, DISCONNECTED};

    Eventloop* m_loop;
    RemoveConnectionCallback m_removeConnectionCallback;
    UserOpenCallback m_openCallback;
    UserCloseCallback m_closeCallback;
    UserMessageCallback m_messageCallback;

    fd_t m_fd;
    InetAddress m_localAddr;
    InetAddress m_peerAddr;
    std::unique_ptr<Channel> m_channel;
    std::unique_ptr<Buffer> m_inputBuffer;
    std::unique_ptr<Buffer> m_outputBuffer;

    state m_state;
};