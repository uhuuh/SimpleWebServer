#pragma once
#include "base.h"


class Activater: public noncopyable {
public:
    Activater(Eventloop* loop);
    ~Activater();
    void activate();
private:
    Eventloop* loop;
    unique_ptr<Channel> ch;
    fd_t fd;
    void handle_read();
};