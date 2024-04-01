#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "Activater.h"
#include "TimerQueue.h"
#include "base.h"
#include <memory>
#include <sys/epoll.h>
#include <unistd.h>
#include "Logger.h"


Eventloop::Eventloop():
    thread_id(gettid())
{
    poller = make_unique<Poller>();

    activater = make_unique<Activater>(this);
    timerQueue = make_unique<TimerQueue>(this);
}

Eventloop::~Eventloop() {
    quit(); 
}

void Eventloop::run() {
    is_loop = true;

    while (is_loop) {
        poller->poll();

        std::vector<Callback> now_cb_list;
        {
            std::lock_guard lock(mu);
            now_cb_list.swap(cb_list); // 使用swap操作减少互斥锁持有的时间
        }
        for (auto& cb: now_cb_list) {
            cb();
        }
    }
}

bool Eventloop::addCallback(Callback cb) {
    INFO("add_cb");
    if (gettid() == thread_id && is_loop) {
        // 有时候loop没启动时，可能添加callback 
        // Eventloop内部调用时的情况，比如timerqueue取消其Channel。如果没有这一个分支，内部调用cb时永远得不到执行
        cb();
        return true;
    } else {
        {
            std::lock_guard lock(mu);
            cb_list.push_back(cb);
        }
        if (is_loop) {
            activater->activate();
        }
    }
    return false;
}

bool Eventloop::isInSameThread() {
    auto now_id = gettid();
    return now_id == thread_id;
}

void Eventloop::quit() {
    is_loop = false;
    activater->activate();
    INFO(format("quit | id: {}", thread_id));
}

void Eventloop::addChannel(Channel* ch) {
    auto cb = [this, ch]() {
        this->poller->addChannel(ch);
        
    };
    addCallback(cb);
}

void Eventloop::removeChannel(Channel* ch) {
    auto cb = [this, ch]() {
        this->poller->removeChannel(ch);
    };
    addCallback(cb);
}

TimerId Eventloop::addTimer(Callback cb, TimeStamp delay_ms) {
    // timerQueue 本身做好了并发处理
    // todo 实现定时器的各种方法，堆实现中如何删除定时器
    auto timer_id = timerQueue->addTimer(cb, delay_ms);
    return timer_id;
}

void Eventloop::cancelTimer(TimerId id) {
    timerQueue->cancelTimer(id);
}
