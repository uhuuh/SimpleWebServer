#pragma once
#include "base.hpp"
#include <memory>

class CoroutinePool;
class CorouteScheduler;
class EventloopPool;
class CoroutineSchedulerEventLoopPool {
public:
    CoroutineSchedulerEventLoopPool(int n_thread, CoroutinePool *co_pool);
    void run();
    void add_callback_to_main(Callback cb);
    void add_callback(Callback cb);
    void stop();
private:
    int i_thread = 0;
    CoroutinePool *co_pool;
    unique_ptr<EventloopPool> loop_pool;
    vector<unique_ptr<CorouteScheduler>> sche_list;
};

class TCPServer2 {
public:
    using UserCoroutineCallback = function<void(int)>;
    UserCoroutineCallback user_cb;
    TCPServer2(const string& ip, int port, int n_thread=1);
    void run();
    void stop();
private:
    void accept_cb();

    const string ip;
    const int port;
    int listen_fd;

    unique_ptr<CoroutinePool> co_pool;
    unique_ptr<CoroutineSchedulerEventLoopPool> co_sche_pool;
    const static int max_conn_num = 20000;
};

