#pragma once
#include <functional>
#include "../logger/Logger.hpp"
using namespace std;



using Callback = function<void(void)>;
class TCPConnection;
class Buffer;

using TCPConnectionCallback = std::function<void(TCPConnection*)>;
struct UserCallback {
    TCPConnectionCallback open_cb;
    TCPConnectionCallback close_cb;
    TCPConnectionCallback message_cb;
    TCPConnectionCallback write_high;
    TCPConnectionCallback write_lower;
};

using CreateConnectionCallback = std::function<void(int, string, int)>;
using RemoveConnectionCallback = std::function<void(void)>;

void assertm(bool res, const char* error_msg);
void assertm(bool res);

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
