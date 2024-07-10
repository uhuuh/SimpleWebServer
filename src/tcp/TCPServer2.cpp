#include "TCPServer2.hpp"
#include "Coroutine.hpp"
#include "EventloopPool.hpp"
#include "co_fun.hpp"


CoroutineSchedulerEventLoopPool::CoroutineSchedulerEventLoopPool(int n_thread, CoroutinePool *co_pool)
    :loop_pool(new EventloopPool(n_thread)) 
{
    sche_list.reserve(n_thread);
    for (int i = 0; i < n_thread; ++i) {
        sche_list[i] = CorouteScheduler(loop_pool->loop_list[i].get(), co_pool);
    }
}

void CoroutineSchedulerEventLoopPool::add_callback_to_main(Callback cb) {
    sche_list[loop_pool->n_thread - 1].add_callback(cb);
}
void CoroutineSchedulerEventLoopPool::add_callback(Callback cb) {
    sche_list[i_thread].add_callback(cb);
    i_thread = (i_thread + 1) % (loop_pool->n_thread - 1);
}
void CoroutineSchedulerEventLoopPool::stop() {
    loop_pool->stop();
}

TCPServer2::TCPServer2(const string &ip, int port, int n_thread)
    : ip(ip), port(port) {
    assertm(n_thread >= 1);
    co_pool = make_unique<CoroutinePool>();
    co_sche_pool =
        make_unique<CoroutineSchedulerEventLoopPool>(n_thread, co_pool.get());
}
void TCPServer2::run() {
    listen_fd = createListenFd(ip, port);
    auto cb = bind(&TCPServer2::accept_cb, this);
    co_sche_pool->add_callback_to_main(cb);
}
void TCPServer2::stop() { co_sche_pool->stop(); }
void TCPServer2::accept_cb() {
    while (true) {
        struct sockaddr_in addr;
        socklen_t len;
        auto peer_fd = co_accept(listen_fd, reinterpret_cast<sockaddr *>(&addr), &len);
        assertm(peer_fd >= 0);

        if (co_pool->co_count() >= max_conn_num) {
            // log
            close(peer_fd);
        };

        auto cb = [peer_fd, this]() { user_cb(peer_fd); };
        co_sche_pool->add_callback(cb);
    }
}