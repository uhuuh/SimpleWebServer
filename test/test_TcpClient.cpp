#include "type.h"
#include "TcpClient.h"
#include "TcpConnection.h"
#include "Connector.h"
#include "EventLoop.h"
#include "ThreadEventloop.h"
#include "ThreadPoolEventloop.h"
#include "Channel.h"
#include "Buffer.h"

int main() {
    TcpClient client("127.0.0.1", 1234);
    auto open = [](TcpConnection* conn) {
        LOG_TRACE << "send before";
        conn->send("hello world");
        LOG_TRACE << "send after";
    };    
    auto message = [](TcpConnection* conn, Buffer* buf) {
        std::cout << buf->peek() << std::endl;
        conn->send(std::string(buf->peek()));
    };
    client.setOpenCallback(open);
    // client.setMessageCallback(message);
    client.run();
}