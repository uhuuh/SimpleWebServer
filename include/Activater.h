#pragma once
#include "EventLoop.h"
#include "base.h"
#include "Channel.h"


class Activater: public Channel {
public:
    Activater(Eventloop* loop);
    void activate();
private:
    void handle_read();
};