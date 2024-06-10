#include "base.h"
#include "TcpClient.h" 
#include "TcpConnection.h"
#include "Buffer.h"
// todo 尝试解决unique_ptr不完整类型


int main() {
    TCPClient client("127.0.0.1", 7000);
    // todo 127.0.0.1 不能绑定

    client.openCallback = [](auto conn) {
        cout << "this is open, " << format("ip: {}, port: {}", conn->peer_ip, conn->peer_port) << endl;
        conn->send("ping pong");
    };
    client.closeCallback = [](auto conn) {
        cout << "this is close, " << format("ip: {}, port: {}", conn->peer_ip, conn->peer_port) << endl;
    };
    client.messageCallback = [](auto conn, auto buf) {
        cout << "<- " << buf->peek() << endl;
        conn->send(string(buf->peek()));
        buf->pop();
    };

    client.run();
    return 0;
}