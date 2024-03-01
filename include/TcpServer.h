#include "type.h"

class TcpServer{
public:
    TcpServer() = delete;
    explicit TcpServer(const char* host, in_port_t port, int n_thread = 4);
    ~TcpServer() = default;

    void setOpenCallback(UserOpenCallback openCallback) {assertm(!m_openCallback); m_openCallback = openCallback;};
    void setCloseCallback(UserCloseCallback closeCallback) {assertm(!m_closeCallback); m_closeCallback = closeCallback;}
    void setMessageCallback(UserMessageCallback messageCallback) {assertm(!m_messageCallback); m_messageCallback = messageCallback;}
    void run();
    void _createConnection(fd_t fd);
    void _removeConnection(fd_t fd);

    std::unique_ptr<Eventloop> m_loop;
    std::unique_ptr<ThreadPoolEventloop> m_loops;
    std::unique_ptr<Acceptor> m_acceptor;
    std::map<fd_t, std::shared_ptr<TcpConnection>> m_connectionMap;

    UserOpenCallback m_openCallback;
    UserCloseCallback m_closeCallback;
    UserMessageCallback m_messageCallback;
};
