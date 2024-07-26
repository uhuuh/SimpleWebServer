#pragma once
#include <functional>
#include <string>
#include "sys_call.hpp"
#include <memory>
#include <list>
#include "Buffer.hpp"
using namespace std;


using Callback = function<void(void)>;



void assertm(bool res, const char* error_msg);
void assertm(bool res);

string to_format_str(const char* format, ...);

uint64_t get_now_timestamp();

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

template <typename T>
class ResourceQueue {
public:
  unique_ptr<T> get(bool is_create=false) {
    if (queue.empty() && is_create) {
      return make_unique<T>();
    } else {
      auto a = std::move(queue.front());
      queue.pop_front();
      return a;
    }
  }
  void add(unique_ptr<T> a) {
    queue.push_back(move(a));
  }
private:
  list<unique_ptr<T>> queue;
};