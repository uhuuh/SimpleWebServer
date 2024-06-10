#pragma once
#include "Eventloop.hpp"
using namespace std;


class Epoll: public EventLoop::Poller {
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
