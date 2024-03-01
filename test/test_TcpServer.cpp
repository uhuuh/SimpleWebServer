#include "type.h"
#include "TcpServer.h"
#include "TcpConnection.h"
#include "ThreadEventloop.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "ThreadPoolEventloop.h"
#include "Buffer.h"


void listen_fd() {
    auto fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    assertm(fd >= 0);
    int reuse = 1;
    assertm(::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) >= 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    serv_addr.sin_port = htons(1234); 
    assertm(::bind(fd, (sockaddr*)&serv_addr, sizeof serv_addr) >= 0);
    assertm(::listen(fd, 5) >= 0);
}

void server() {
    std::cout << "ptr size: " << sizeof(void*) << std::endl;
    TcpServer server("127.0.0.1", 1234);
    auto message = [](TcpConnection* conn, Buffer* buf) {
        std::cout << buf->peek() << std::endl;
        conn->send(std::string(buf->peek()));
        buf->pop();
    };
    server.setMessageCallback(message);
    server.run();
}

int main() {
    // listen_fd();
    server();
    return 0;
}