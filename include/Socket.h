#pragma once
#include "InetAddress.h"
using fd_t = int;

class Socket {
public:
    static InetAddress getLocalAddress(fd_t fd);
    static InetAddress getPeerAddress(fd_t fd);
};

