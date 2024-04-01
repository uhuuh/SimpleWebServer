#include "EventloopPool.h"
#include "EventLoop.h"
#include "Channel.h"
#include <thread>
#include <chrono>
#include <unistd.h>
#include "Logger.h"


int main() {
    EventloopPool pool(1);
    Eventloop* loop = pool.getLoop();

    auto cb = []() {
        cout << "this is timer" << endl;
    };
    loop->addTimer(cb, 3000);

    Channel ch(loop, 0);
    auto cb2 = []() {
        char input[100];
        read(0, &input, sizeof(input));
        cout << "recv: " << input << endl;
    };
    ch.addEvent(EventType::READ, cb2);

    auto cb3 = []() {
        cout << "this is cb" << endl;
    };
    loop->addCallback(cb3);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
    };

    return 0;
}
