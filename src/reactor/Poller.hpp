#pragma once
#include "Eventloop.hpp"
using namespace std;

class Poller {
public:
    virtual void update_channel(EventLoop::Channel*) = 0;
    virtual void remove_channel(EventLoop::Channel*) = 0;
    virtual void poll() = 0;
};

class Epoll: public Poller {
public:
    Epoll();
    ~Epoll();
    void update_channel(EventLoop::Channel* ch) override;
    void remove_channel(EventLoop::Channel* ch) override;
    void poll() override;
private:
    class Impl;
    Impl* impl;
};
