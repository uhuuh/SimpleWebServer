#pragma once
#include <string_view>
#include <unistd.h>
#include <vector>
using namespace std;


// 实际上是一个可以扩容的队列, 右边插入, 左边弹出. 当插入容量不够时, 左边有空余移到左边, 否则resize
class Buffer {
public:
    Buffer();
    ~Buffer() = default;

    int push_from(int fd, decltype(read) io_fun=nullptr);
    void push(const string_view str);
    int pop_to(int fd, decltype(write) io_fun=nullptr);
    void pop(int len);
    void pop();
    string_view peek();
    int get_size();

private:
    char* begin();
    void buf_move_head();
    void buf_insert_tail(const string_view str);

    int left_read_ptr;
    int right_write_ptr;
    vector<char> buf;
    static const int init_capa = 1024;
};


