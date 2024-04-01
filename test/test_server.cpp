#include "base.h"
#include "TcpServer.h" 
// todo 尝试解决unique_ptr不完整类型


int main() {
    TcpServer server("0.0.0.0", 7000);
    // todo 127.0.0.1 不能绑定
    server.openCallback = [](auto conn) {
        cout << "this is open, " << format("ip: {}, port: {}", conn->peer_ip, conn->peer_port) << endl;
    };
    server.closeCallback = [](auto conn) {
        cout << "this is close, " << format("ip: {}, port: {}", conn->peer_ip, conn->peer_port) << endl;
    };
    server.messageCallback = [](auto conn, auto buf) {
        cout << "<- " << buf->peek() << endl;
        conn->send(string(buf->peek()));
        buf->pop();
    };

    server.run();
    return 0;
}