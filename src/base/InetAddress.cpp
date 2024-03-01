#include "InetAddress.h"
#include "iostream"

InetAddress::InetAddress() {
    memset(&m_addr, 0, sizeof(m_addr));
}

InetAddress::InetAddress(const std::string& host, in_port_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = ::inet_addr(host.c_str());
    m_addr.sin_port = ::htons(port);
}

InetAddress::InetAddress(in_port_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    m_addr.sin_port = ::htons(port);
}

std::string InetAddress::toString() {
    char ip_str[INET_ADDRSTRLEN];
    auto err = inet_ntop(AF_INET, &(m_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
    if (err == NULL) {
        std::cout << "toString fail" << std::endl;
        return "";
    }
    return std::string(ip_str) + ":" + std::to_string(ntohs(m_addr.sin_port));
}



