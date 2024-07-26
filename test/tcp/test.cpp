#include <thread>
#include "Logger.hpp"
#include "base.hpp"
#include "tcp_user.hpp"
using namespace std;

void test_server() {
    auto server_cb = [] () {
        int count = 100;
        TCPServer server("0.0.0.0", 8000, 1);
        auto open_cb = [] (TCPConnection* conn) {
            printf("server open conn, from ip=%s port=%d fd=%d\n",
                conn->peer_ip.c_str(), 
                conn->peer_port, 
                conn->peer_fd
            );
        };
        auto close_cb = [] (TCPConnection *conn) {
            printf("server close conn from ip=%s port=%d fd=%d\n", 
                conn->peer_ip.c_str(),
                conn->peer_port,
                conn->peer_fd
            );
        };
        auto message_cb = [&] (TCPConnection *conn) {
            printf("server conn fd=%d recv: %s\n", conn->peer_fd, conn->get_data().begin());
            conn->send_data(string(conn->get_data()));
            conn->pop_data();
            count -= 1;
            if (count < 0) {
                conn->half_close();
                // server.stop();
            }
        };
        server.user_cb.open_cb = open_cb;
        server.user_cb.close_cb = close_cb;
        server.user_cb.message_cb = message_cb;
        server.run();
    };
    auto client_cb = [] () {
        TCPClient client("127.0.0.1", 8000);
        auto open_cb = [] (TCPConnection* conn) {
            printf("client open conn, from ip=%s port=%d fd=%d\n",
                conn->peer_ip.c_str(), 
                conn->peer_port, 
                conn->peer_fd
            );
            conn->send_data("hello world");
        };
        auto close_cb = [&] (TCPConnection *conn) {
            printf("client close conn from ip=%s port=%d fd=%d\n", 
                conn->peer_ip.c_str(),
                conn->peer_port,
                conn->peer_fd
            );
        };
        auto message_cb = [] (TCPConnection *conn) {
            printf("client conn fd=%d recv: %s\n", conn->peer_fd, conn->get_data().begin());
            conn->send_data(conn->get_data().begin());
            conn->pop_data();
        };
        client.user_cb.open_cb = open_cb;
        client.user_cb.close_cb = close_cb;
        client.user_cb.message_cb = message_cb;

        client.run();
    };

    thread t_server(server_cb);
    thread t_client_1(client_cb);
    thread t_client_2(client_cb);

    t_server.join();
    t_client_1.join();
    t_client_2.join();
}

void test_server2() {
    // Logger::get_instance()->is_print = true;
    cout << "test server2" << endl;
    // telnet 127.0.0.1 7000
    TCPServer2 server("0.0.0.0", 7000, 3);
    auto cb = [&] (int fd) {
        const int buf_size = 1024;
        char buf[buf_size];
        int n = 0;
        for (int i = 0; i < 5; ++i) {
            assertm(n = co_read(fd, buf, buf_size));
            co_write(fd, buf, n);
        }
        co_close(fd);
        LOG_DEBUG("close fd");
        server.stop();
    };
    server.user_cb = cb;
    server.run();

    printf("end\n");
};

int main() {
    // test_server();
    test_server2();

    return 0;
}