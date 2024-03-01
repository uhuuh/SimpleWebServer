#pragma once
#include "type.h"

class TcpClient {
public:
    TcpClient(const std::string& host, in_port_t port);

    void setOpenCallback(UserOpenCallback openCallback) {assertm(!m_openCallback); m_openCallback = openCallback;};
    void setCloseCallback(UserCloseCallback closeCallback) {assertm(!m_closeCallback); m_closeCallback = closeCallback;}
    void setMessageCallback(UserMessageCallback messageCallback) {assertm(!m_messageCallback); m_messageCallback = messageCallback;}
    void run();
    void _createConnection(fd_t fd);
    void _removeConnection(fd_t fd);

    std::unique_ptr<Eventloop> m_loop;
    std::unique_ptr<Connector> m_connector;
    std::unique_ptr<TcpConnection> m_connection;

    UserOpenCallback m_openCallback;
    UserCloseCallback m_closeCallback;
    UserMessageCallback m_messageCallback;
};
