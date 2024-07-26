#pragma once
#include "Eventloop.hpp"
#include "base.hpp"
#include <memory>
#include <unordered_map>
using namespace std;

extern "C" {

struct Coroutine;
extern void co_swap(Coroutine*, Coroutine*) asm("co_swap");

};

struct Coroutine {
  struct regs_t {
    void* r15;
    void* r14;
    void* r13;
    void* r12;
    void* r9;
    void* r8;
    void* rbp;
    void* rdi;
    void* rsi;
    void* ret;
    void* rdx;
    void* rcx;
    void* rbx;
    void* rsp;
  } regs;
  const size_t st_size = 128 * 1024;
  vector<char> st;
  Coroutine* pre_co = nullptr;
  Callback co_cb = nullptr;

  void set_callback(Callback cb);
  static void co_fun(Coroutine* co);
  void resume(Coroutine* next_co);
  void yield();
};



class CoroutinePool {
public:
  class Channel {
  public:
    Channel(unique_ptr<Coroutine> co, CoroutinePool* co_pool): 
      co(std::move(co)), co_pool(co_pool) {}
    Coroutine* get() {
      return co.get();
    }
    ~Channel() {
      co_pool->queue.add(std::move(co));
      co_pool->count -= 1;
    }
  private:
    unique_ptr<Coroutine> co;
    CoroutinePool *co_pool;
  };
  unique_ptr<Channel> get_channel() {
    lock_guard<mutex> lock(mu);
    count += 1;
    return make_unique<Channel>(queue.get(true), this);
  } 
  int co_count() {
    return count;
  }
private:
  int count = 0;
  mutex mu;
  ResourceQueue<Coroutine> queue;
};


class EventLoop;
class CorouteScheduler {
public:
  CorouteScheduler(EventLoop* loop, CoroutinePool* co_pool);
  void add_callback(Callback cb);
  void schedule_co(Coroutine* co=nullptr);

  bool is_schedule = false;
  EventLoop* loop;
  CoroutinePool* co_pool;
  unordered_map<int, unique_ptr<EventLoop::Channel>> fd_map;
  unordered_map<void*, unique_ptr<CoroutinePool::Channel>> co_map;
  unique_ptr<Coroutine> main_co;
  Coroutine* now_co;
  list<Coroutine*> ready_queue;
private:
  void handle_co();
};

