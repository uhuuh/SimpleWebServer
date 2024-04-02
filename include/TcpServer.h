#include "base.h"
#include "EventloopPool.h"
#include "Acceptor.h"
#include "TcpConnection.h"
using namespace std;


class TcpServer: public noncopyable {
public:
    TcpServer(const string& ip, port_t port, int n_thread = 0);
    ~TcpServer() = default;

    void run();

    UserOpenCallback openCallback;
    UserCloseCallback closeCallback;
    UserMessageCallback messageCallback;
private:
    void create_conn(fd_t fd, const string peer_ip, const int peer_port);
    void remove_conn(fd_t fd);

    const string ip;
    const port_t port;
    unique_ptr<EventloopPool> loop_pool;
    Eventloop* main_loop;
    unique_ptr<Acceptor> acceptor;
    map<fd_t, std::unique_ptr<TcpConnection>> conn_map;
};
