#include <arpa/inet.h>
#include <cassert>
#include <csignal>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
using namespace std;


int createListenFd(const string& ip, int port) {
    auto fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    assert(fd >= 0);

    int reuse = 1;
    assert(::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) >= 0); // 设置复用

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    // address.sin_addr.s_addr = INADDR_ANY; // 监听所有网络接口
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    addr.sin_port = htons(port);

    assert(bind(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) >= 0);
    assert(listen(fd, SOMAXCONN) >= 0); 

    return fd;
}

void blockSIGPIPE() {
    sigset_t new_set;
    sigemptyset(&new_set);
    sigaddset(&new_set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &new_set, nullptr);
}