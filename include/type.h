#pragma once
#include "iostream"
#include "unistd.h"
#include <memory>
#include <cstdint>
#include <utility>
#include <functional>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <netinet/in.h>
#include "sys/select.h"
#include "unistd.h"
#include <condition_variable>

#include "Logger.h"
#include "InetAddress.h"
#include "Timer.h"
#include "Socket.h"

// #include "easylogging++.h"
#include <thread>
#include "mutex"


class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

using fd_t = int;


class InetAddress;
class Socket;
class Buffer;
class Logger;
class Channel;
class Eventloop;
class ThreadPoolEventloop;
class Poller;
class ConnectionContext;
class AcceptContext;
class TcpServer;
class Activater;
class TcpConnection;
class ThreadEventloop;
class Acceptor;
class TcpConnection;
class TimerQueue;
class Connector;

using SocketPtr = std::unique_ptr<Socket>;
using ChannelPtr = std::shared_ptr<Channel>;
using ChannelList = std::vector<std::shared_ptr<Channel>>;
using ChannelMap = std::map<fd_t, std::shared_ptr<Channel>>;

using Callback = std::function<void()>;

using NewConnectionCallback = std::function<void(fd_t)>;
using RemoveConnectionCallback = std::function<void(fd_t)>;
using UserCallback = std::function<void(ConnectionContext&)>;

using UserOpenCallback = std::function<void(TcpConnection*)>;
using UserCloseCallback = std::function<void(TcpConnection*)>;
using UserMessageCallback = std::function<void(TcpConnection*, Buffer*)>;





