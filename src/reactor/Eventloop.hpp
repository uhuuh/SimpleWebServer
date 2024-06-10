#pragma once
#include <cstdint>
#include <unistd.h>
#include <vector>
#include "base.hpp"
#include "TimerQueue.hpp"


class EventLoop: noncopyable {
public:
    class Channel;
    class Poller;
    enum class EventType;

    EventLoop();
    ~EventLoop();
    void run();
    void quit();
    bool is_same_thread();
    Channel* get_channel(int fd);
    bool add_callback(Callback cb);
    void add_callback_now(Callback cb);
    void add_callback_after(Callback cb);
    TimerQueue::TimerId add_timer(Callback cb, uint64_t delay_ms);
    void remove_timer(TimerQueue::TimerId id);
private:
    void update_channel(Channel*);
    void remove_channel(Channel*);
    class Impl;
    Impl* impl;
};

enum class EventLoop::EventType {
    READ,
    WRITE,
    size
};

class EventLoop::Channel {
public:
    Channel(EventLoop* loop, int fd)
        : loop(loop), fd(fd) {
        assertm(loop != nullptr);
        assertm(fd >= 0);
    }
    ~Channel() {
        loop->remove_channel(this);
    }
    void set_event(EventType et, Callback cb) {
        int idx = static_cast<int>(et);
        enable_arr[idx] = true;
        cb_arr[idx] = cb;
        loop->update_channel(this);
    }
    void enable_event(EventType et) {
        int idx = static_cast<int>(et);
        enable_arr[idx] = true;
        loop->update_channel(this);
    }
    void disable_event(EventType et) {
        int idx = static_cast<int>(et);
        enable_arr[idx] = false;
        loop->update_channel(this);
    }
    void handle_event() {
        for (int i = 0; i < trigger_arr.size(); ++i) {
            if (trigger_arr[i]) {
                cb_arr[i]();
                trigger_arr[i] = false;
            }
        }
    }
    inline bool is_enable(EventType et) {
        int idx = static_cast<int>(et);
        return enable_arr[idx];
    }
    inline void trigger_event(EventType et) {
        int idx = static_cast<int>(et);
        trigger_arr[idx] = true;
    }
    inline int get_fd() {
        return fd;
    }
private:
    EventLoop* loop;
    const int fd = -1;
    vector<Callback> cb_arr{static_cast<int>(EventType::size)};
    vector<bool> trigger_arr = vector<bool>(static_cast<int>(EventType::size));
    vector<bool> enable_arr = vector<bool>(static_cast<int>(EventType::size));
};

class EventLoop::Poller {
public:
    virtual void update_channel(Channel*) = 0;
    virtual void remove_channel(Channel*) = 0;
    virtual void poll() = 0;
    // virtual ~Poller() = 0;
};