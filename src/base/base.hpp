#pragma once
#include <functional>
using namespace std;


using Callback = function<void(void)>;

class TCPConnection;
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

string to_format_str(const char* format, ...);

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};


