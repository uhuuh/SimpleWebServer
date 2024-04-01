#include <map>
#include <iostream>
#include <functional>
#include <unistd.h>
#include <thread>
using namespace std;

int main() {
    auto cb = []() {
        cout << gettid() << endl;
    };
    auto t1 = thread(cb);
    auto t2 = thread(cb);
    auto t3 = thread(cb);
    t1.join();
    t2.join();
    t3.join();
    return 0;
}

