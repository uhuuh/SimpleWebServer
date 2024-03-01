#pragma once
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

class InetAddress {
public:
    InetAddress();
    explicit InetAddress(in_port_t port);
    InetAddress(const std::string &host, in_port_t port);
    ~InetAddress() = default;
    struct sockaddr_in m_addr;
    std::string toString();
};
