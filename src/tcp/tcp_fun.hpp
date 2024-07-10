#pragma once
#include "base.hpp"

class TCPConnection;
using TCPConnectionCallback = function<void(TCPConnection*)>;
using CreateConnectionCallback = function<void(int, string, int)>;
using RemoveConnectionCallback = function<void(void)>;

struct UserCallback {
    TCPConnectionCallback open_cb;
    TCPConnectionCallback close_cb;
    TCPConnectionCallback message_cb;
    TCPConnectionCallback write_high;
    TCPConnectionCallback write_lower;
};
