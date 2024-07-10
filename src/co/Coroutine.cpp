#include "Coroutine.hpp"
#include "Eventloop.hpp"
#include "base.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <list>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

void Coroutine::set_callback(Callback cb) {
    memset(&regs, 0, sizeof(regs));
    st.resize(st_size);
    pre_co = nullptr;
    co_cb = cb;

    // sp显示是栈底地址，实现16字节对齐
    auto st_ptr = reinterpret_cast<char *>(&st.front());
    char *sp = st_ptr + st_size - sizeof(void *);
    sp =
        (char
             *)((unsigned long)sp &
                -16LL); // -16补码表示是11111111111111111111111111110000，与操作后实现16字节对齐

    // 栈底提前保存回调地址pfn，要不然可能段错误
    // todo 这里实际上也应该可以设定其他地址，来实现到其他协程的跳转
    // todo 这里设定好像没什么用
    void **ret_addr = (void **)(sp);
    // *ret_addr = (void *)pfn;
    *ret_addr = 0;

    regs.rsp = sp;
    regs.ret = (char *)Coroutine::co_fun;
    // 传给 pfn的第一个和第二个参数
    regs.rdi = (char *)this;
    // regs.rsi = (char *)pfn;
}
void Coroutine::co_fun(Coroutine *co) {
    co->co_cb();
    co->yield(); // 回调结束后可以自动跳转到协程调用处执行
}
void Coroutine::resume(Coroutine *next_co) {
    next_co->pre_co = this;
    co_swap(this, next_co);
}
void Coroutine::yield() {
    assert(pre_co != nullptr);
    co_swap(this, pre_co);
}

extern void co_env_init(CorouteScheduler *);
CorouteScheduler::CorouteScheduler(EventLoop *loop, CoroutinePool *co_pool)
    : loop(loop), co_pool(co_pool), main_co(new Coroutine()) {
    auto cb = [this]() { co_env_init(this); };
    loop->add_callback(cb);
    // main_co 不需要绑定回调
}
void CorouteScheduler::add_callback(Callback cb) {
    shared_ptr<CoroutinePool::Channel> co_ch = co_pool->get_channel();
    auto co_p = co_ch->get();

    // 如果没有mutable，co_ch是const share_ptr，reset不能调用成功
    auto cb2 = [cb, co_ch]() mutable {
        cb();
        // 这里销毁co_ch
        co_ch.reset();
    };
    co_p->set_callback(cb2);

    ready_queue.push_back(co_p);
    LOG_DEBUG("add_callback %p", co_p);

    schedule_co();
}

void CorouteScheduler::schedule_co(Coroutine *co) {
    if (co != nullptr) {
        ready_queue.push_back(co);
    }

    if (!is_schedule) {
        auto cb = bind(&CorouteScheduler::handle_co, this);
        // loop->add_callback(cb); // 立刻执行handle_co，使得is_schedule为true
        loop->add_callback_after(cb);
        is_schedule = true;
        LOG_DEBUG("schedule co %p, is_schedule=%d", co, int(is_schedule));
    }
}

void CorouteScheduler::handle_co() {

    while (!ready_queue.empty()) {
        now_co = ready_queue.front();
        ready_queue.pop_front();
        LOG_DEBUG("handle co %p", now_co);
        main_co->resume(now_co);
    }

    is_schedule = false;
    LOG_DEBUG("handle co end, is_schedule=%d", int(is_schedule));
}
