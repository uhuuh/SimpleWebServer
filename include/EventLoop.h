#pragma once
#include "base.h"


class Eventloop: noncopyable {
public:
    Eventloop();
    ~Eventloop();

    void run();
    void quit();

    // 默认参数用法
    void addCallback(Callback cb);

    TimerId addTimer(Callback cb, TimeStamp delay_ms);
    void cancelTimer(TimerId id);

    bool is_loop;
private:
    // 实现Channel才能使用下面这些函数
    // 添加修改Channel的函数还是loop实现比Channel实现好，因为是loop修改自身
    // todo 友元是如何实现的，java中为什么没有友元，什么场景下使用友元
    friend class Channel;
    void addChannel(Channel* ch);
    void updateChannel(Channel* ch);
    void removeChannel(Channel *ch);

    std::mutex mu;
    int thread_id;
    unique_ptr<Poller> poller;
    unique_ptr<Activater> activater;
    unique_ptr<TimerQueue> timerQueue;
    vector<Callback> cb_list;
    static const uint32_t timeout_ms = 10;
};

