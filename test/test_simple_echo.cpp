#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
using namespace std;  

constexpr int BUFFER_SIZE = 1e6;
constexpr int PORT = 7000;
char buffer[BUFFER_SIZE];

int main() {
    // 创建 TCP 套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "Setsockopt failed\n";
        return 1;
    }

    // 准备地址结构
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 绑定套接字到地址和端口
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    // 开始监听连接
    if (listen(server_fd, 5) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Echo server is listening on port " << PORT << std::endl;

    while (true) {
        // 接受客户端连接
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            std::cerr << "Accept failed\n";
            continue;
        }

        ssize_t bytes_received;
        bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received == 0) {
            // 客户端关闭了连接
            std::cout << "Client closed connection\n";
        } else if (bytes_received < 0) {
            std::cerr << "Receive error\n";
        }

        buffer[bytes_received] = 0;
        cout << buffer << endl;

        send(client_fd, buffer, bytes_received, 0);

        // close(client_fd); // 关闭客户端套接字
        // cout << "before" << endl;
        // shutdown(client_fd, SHUT_RD);
        // shutdown(client_fd, SHUT_WR);
        // shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
        // cout << "after" << endl;
        // bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        // cout << "read comp, "  << bytes_received << endl;
    }

    close(server_fd); // 关闭服务器套接字

    return 0;
}
