#pragma once

#include <cstdint>
#include <iostream>
#include <cstdio>

#include <string>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <queue>
#include <algorithm>

#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

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

using CreateConnectionCallback = std::function<void(fd_t, string, int)>;
using RemoveConnectionCallback = std::function<void(void)>;

using TimeStamp = uint64_t;
using TimerSeq = uint64_t;
class Timer;
struct TimerId {
    TimeStamp ms;
    TimerSeq seq;
    Timer* timer;
};

// using TimerId = std::multimap<TimeStamp, Callback>::iterator;

