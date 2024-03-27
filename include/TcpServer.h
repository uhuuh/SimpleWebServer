#include "base.h"
using namespace std;


class TcpServer: public noncopyable {
public:
    TcpServer(const string& ip, port_t port, int n_thread);
    ~TcpServer() = default;

    void run();

    UserOpenCallback openCallback;
    UserCloseCallback closeCallback;
    UserMessageCallback messageCallback;
private:
    void create_conn(fd_t fd, const string& peer_ip, const int peer_port);
    void remove_conn(fd_t fd);

    const string ip;
    port_t port;
    unique_ptr<EventloopPool> loop_pool;
    Eventloop* loop;
    std::unique_ptr<Acceptor> acceptor;
    std::map<fd_t, std::unique_ptr<TcpConnection>> conn_map;
    list<fd_t> remove_conn_set;
};
