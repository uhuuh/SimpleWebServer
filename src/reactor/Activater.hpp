#pragma once

class EventLoop;

class Activater {
public:
    Activater(EventLoop* loop);
    ~Activater();
    void activate();
private:
    class Impl;
    Impl* impl;
};
