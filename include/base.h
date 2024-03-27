#pragma once

#include <iostream>
#include <cstdio>

#include <memory>
#include <functional>

#include <string>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <algorithm>


#include <thread>
#include <mutex>

#include "Logger.h"

using namespace std;

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
using port_t = uint16_t;

class Eventloop;
class EventloopPool;
class Channel;
class Poller;
class Activater;
class TimerQueue;

class Buffer;
class Connector;
class Acceptor;
class TcpConnection;

using Callback = std::function<void()>;

using UserOpenCallback = std::function<void(TcpConnection*)>;
using UserCloseCallback = std::function<void(TcpConnection*)>;
using UserMessageCallback = std::function<void(TcpConnection*, Buffer*)>;

using CreateConnectionCallback = std::function<void(fd_t, const string&, const int)>;
using RemoveConnectionCallback = std::function<void(fd_t)>;

using TimeStamp = uint64_t;
using TimerId = std::multimap<TimeStamp, Callback>::iterator;


