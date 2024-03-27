#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "Activater.h"
#include "TimerQueue.h"
#include <sys/epoll.h>
#include <unistd.h>


Eventloop::Eventloop()
{
    activater.reset(new Activater(this));
    timerQueue.reset(new TimerQueue(this));
    thread_id = gettid();
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

void Eventloop::addCallback(Callback cb) {
    if (gettid() == thread_id) {
        // note Eventloop内部调用时的情况，比如timerqueue取消其Channel
        cb();
    } else {
        {
            std::lock_guard lock(mu);
            cb_list.push_back(cb);
        }

        activater->activate();
    }
}

void Eventloop::quit() {
    is_loop = false;
    activater->activate();
}

void Eventloop::addChannel(Channel* ch) {
    auto cb = [this, ch]() {
        this->poller->addChannel(ch);
    };
    addCallback(cb);
}

void Eventloop::updateChannel(Channel* ch) {
    auto cb = [this, ch]() {
        this->poller->updateChannel(ch);
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
    // todo 并发处理
    return timerQueue->addTimer(cb, delay_ms);
}

void Eventloop::cancelTimer(TimerId id) {
    auto cb = [this, id] () {
        timerQueue->cancelTimer(id);
    };
    addCallback(cb);
}
