#include "Coroutine.hpp"
#include "Logger.hpp"
#include <cassert>
#include <cstdint>
#include <sys/select.h>


thread_local CorouteScheduler* thread_co_sche = nullptr;

void co_env_init(CorouteScheduler* co_sche) {
  assert(thread_co_sche == nullptr);
  thread_co_sche = co_sche;
  LOG_DEBUG("thread_co_sche %p", co_sche);
}

void co_yield_() {
  thread_co_sche->now_co->yield();
}

template<typename Fun, typename EventType>
ssize_t _co_io_fun(const Fun& f, int fd, EventType et) {
  if (thread_co_sche->fd_map.find(fd) == thread_co_sche->fd_map.end()) {
    auto ch = thread_co_sche->loop->get_channel(fd);
    // ch设定为once，不需要关闭对应事件
    ch->is_once = true;
    thread_co_sche->fd_map.insert({fd, std::move(ch)});
  }

  auto ret = f();
  if (ret >= 0) return ret;

  auto &ch = thread_co_sche->fd_map[fd];
  if (!ch->is_enable(et)) {
    auto cb = [fd, co=thread_co_sche->now_co] () {
      thread_co_sche->schedule_co(co);
    };
    ch->enable_event(et, cb);
  } else {
    ch->enable_event(et);
  }

  co_yield_();
  return f();
}


ssize_t co_read(int fd, void* buf, size_t size) {
  auto fun = [fd, buf, size] { return read(fd, buf, size); };
  return _co_io_fun(fun, fd, EventLoop::EventType::READ);
}

ssize_t co_write(int fd, const void* buf, size_t size) {
  auto fun = [fd, buf, size] { return write(fd, buf, size); };
  return _co_io_fun(fun, fd, EventLoop::EventType::WRITE);
}

// ssize_t coroutine_connect(int fd, const struct sockaddr * addr, socklen_t len) {
//   auto fun = [fd, addr, len] { return connect(fd, addr, len); };
//   return _coroutine_io_fun(fun, fd, EventLoop::EventType::READ);
// }

ssize_t co_accept(int fd, struct sockaddr* addr, socklen_t* len) {
  assertm(thread_co_sche->loop->is_same_thread());
  auto fun = [fd, addr, len] { return accept(fd, addr, len); };
  return _co_io_fun(fun, fd, EventLoop::EventType::READ);
}

void co_sleep(int ms) {
  auto cb = [co=thread_co_sche->now_co] () { 
    thread_co_sche->schedule_co(co);
  };

  thread_co_sche->loop->add_timer({static_cast<uint64_t>(ms), cb});
  thread_co_sche->now_co->yield();
}

void co_close(int fd) {
  // 清除fd对应的ch
  thread_co_sche->fd_map.erase(fd);
  // close(fd); // ch析构的时候会关闭fd
}