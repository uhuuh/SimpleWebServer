#include <condition_variable>
#include <list>
#include <mutex>
using namespace std;

// 多生产者，多消费者
template <typename T>
class BlockQueue {
    list<T> queue;
    mutex mu;
    condition_variable conv;
public:
    void push(T& a) {
        lock_guard<mutex> lock(mu);
        queue.push_back(a);
        conv.notify_one();
    }
    T pop() {
        // 与信号量实现不同，信号量PV后对queue修改还需要另外一个互斥锁。但是使用条件变量实现时，wait后会加锁
        unique_lock<mutex> mu;
        if (queue.size() == 0) {
            conv.wait(lock, [&](){queue.size() > 0;});
        }
        auto a = queue.front();
        queue.pop_front();
        return a;
    }
};

template <typename T>
class FixedBlockQueue {
    list<T> queue;
    const int capacity;
    mutex mu;
    condition_variable conv_not_empty;
    condition_variable conv_not_full;
public:
    explicit FixedBlockQueue(int capacity): capacity(capacity) {}
    void push(T& a) {
        unique_lock<mutex> lock(mu);
        if (queue.size() == capacity) {
            conv_not_full.wait(lock, [&](){queue.size() < capacity;});
        }
        conv_not_empty.notify_one(); // 与非固定阻塞队列相比，这里要注意
        queue.push_back(a);
    }
    T pop() {
        unique_lock<mutex> lock(mu);
        if (queue.size() == 0) {
            conv_not_empty.wait(lock, [&](){queue.size() > 0;});
        }
        auto a = queue.front();
        queue.pop_front();
        conv_not_full.notify_one();
        return a;
    }
};
