#pragma once
#include <cstdint>
#include <memory>
#include <mutex>
#include <unistd.h>
#include <vector>
#include "Activater.hpp"
#include "Logger.hpp"
#include "base.hpp"
#include "TimerScheduler.hpp"


class TimerSchedulerAdapter;
class Poller;
class EventLoop: noncopyable {
public:
    class Channel;
    enum class EventType;

    EventLoop();
    ~EventLoop();
    void run();
    void stop();
    bool is_same_thread();
    bool is_run();
    unique_ptr<Channel> get_channel(int fd);
    bool add_callback(Callback cb);
    void add_callback_now(Callback cb);
    void add_callback_after(Callback cb);
    TimerId add_timer(Timer ti);
    void remove_timer(TimerId id);
private:
    void update_channel(Channel*);
    void remove_channel(Channel*);


    bool is_loop = false;
    int thread_id;
    mutex mu;
    unique_ptr<Poller> poller; // poller必须放后面，它的析构函数需要后调用，最后才关闭 epoll fd
    unique_ptr<Activater> activater;
    unique_ptr<TimerSchedulerAdapter> ti_sche;
    vector<Callback> cb_list;
    static const uint32_t timeout_ms = 10;
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
        close(fd);
        // LOG_TRACE("channel destroy, %s", to_str().c_str());
    }
    void set_event(EventType et, const Callback& cb) {
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
    string to_str() {
        return to_format_str("fd=%d enable_read=%d enable_write=%d", fd, (int)enable_arr[0], (int)enable_arr[1]);
    }
    bool is_once = false;
private:
    EventLoop* loop;
    const int fd = -1;
    vector<Callback> cb_arr{static_cast<int>(EventType::size)};
    vector<bool> trigger_arr = vector<bool>(static_cast<int>(EventType::size));
    vector<bool> enable_arr = vector<bool>(static_cast<int>(EventType::size));
};
