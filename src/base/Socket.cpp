#include "Socket.h"
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/socket.h>

InetAddress Socket::getLocalAddress(fd_t fd) {
    InetAddress addr;
    auto ptr = reinterpret_cast<sockaddr*>(&addr); 
    socklen_t len = sizeof(sockaddr_in);
    ::getsockname(fd, ptr, &len);
    return addr;
}

InetAddress Socket::getPeerAddress(fd_t fd) {
    InetAddress addr;
    auto ptr = reinterpret_cast<sockaddr*>(&addr); 
    socklen_t len = sizeof(sockaddr_in);
    ::getpeername(fd, ptr, &len);
    return addr;
}