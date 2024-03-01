#include <iostream>
#include "ThreadEventloop.h"
#include "Eventloop.h"
#include "Channel.h"
#include <thread>
#include "type.h"
#include <sys/timerfd.h>
#include <time.h>
#include "Activater.h"
#include "sys/eventfd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include "Poller.h"
#include "TcpServer.h"
#include "Acceptor.h"
#include "ThreadPoolEventloop.h"


void two_loop() {
    Eventloop loop;
    check_execption([]{Eventloop loop;}, "correct", "error");
}

void call_in_other_thread() {
    ThreadEventloop tloop;
    auto loop = tloop.getLoop();

    check_execption([&]{loop->assertSameThread();}, "correct", "error");
}


void threadloop_normal_use() {
    ThreadEventloop tloop;
    auto loop = tloop.getLoop();
    check_execption([&]{loop->assertSameThread();}, "", "");

    Channel channel(0);
    auto fun = [] {std::cout << "event" << std::endl;};
    channel.addEvent(EventType::READ, fun);
    loop->updateChannel(&channel, false);
    sleep(10);
}

void loop_time() {
    Eventloop loop;

    auto fd = timerfd_create(CLOCK_REALTIME, 0);
    struct itimerspec  new_value;
    memset(&new_value, 0, sizeof(new_value));
    new_value.it_value.tv_sec = 1;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = 1;
    new_value.it_value.tv_nsec = 0;
    timerfd_settime(fd, 0, &new_value, NULL);
    uint64_t exp;

    Channel channel(fd);
    auto fun = [&] {
        read(fd, &exp, sizeof(exp));
        std::cout << "event" << std::endl;
    };
    channel.addEvent(EventType::READ, fun);
    loop.updateChannel(&channel, false);

    loop.loop();
}

void loop_activate() {
    Eventloop loop;

    auto fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    uint64_t one = 1;
    ::write(fd, &one, sizeof one);
    Channel channel(fd);
    auto handleRead = [&] {
        std::cout << "event" << std::endl;
        uint64_t one = 1;
        ::read(fd, &one, sizeof one);
        // ::write(fd, &one, sizeof one);
    };
    channel.addEvent(EventType::READ, handleRead);
    loop.updateChannel(&channel, true);

    loop.loop();
}

void quit_activate() {
    std::unique_ptr<ThreadEventloop> ptr = std::make_unique<ThreadEventloop>();
    ptr->getLoop();

    auto start_time = std::chrono::high_resolution_clock::now();
    ptr.reset();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << duration.count() << std::endl;
    check(duration.count() < 10, "error");
}

void timerfd() {
    struct timespec now;
    assertm(clock_gettime(CLOCK_REALTIME, &now) >= 0);
    printf("now, %ld, %ld\n", now.tv_sec, now.tv_nsec);
    auto timestamp = getNowTimeStamp();
    printf("now, %ld, %ld\n", timestamp / int(1e3), (timestamp % int(1e3)) * int(1e6));

    struct itimerspec  new_value;
    new_value.it_value.tv_sec = timestamp / int(1e3);
    new_value.it_value.tv_nsec = (timestamp % int(1e3)) * int(1e6);
    new_value.it_interval.tv_sec = 1;
    new_value.it_interval.tv_nsec = 0;

    auto fd = ::timerfd_create(CLOCK_REALTIME, 0);
    // auto fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    assertm(fd >= 0);
    assertm(timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) >= 0);

    ::sleep(2);

    Poller poller;
    Channel channel(fd);
    auto fun = [] {std::cout << "event" << std::endl;};
    channel.addEvent(EventType::READ, fun);
    poller.updateChannel(&channel);
    std::vector<Channel*> channels;
    std::cout << getNowTimeStamp() << std::endl;
    poller.poll(5, &channels);
    std::cout << getNowTimeStamp() << std::endl;
    std::cout << channels.size() << std::endl;

    // int max_exp = 5;
    // uint64_t exp;
    // for (int tot_exp = 0; tot_exp < max_exp; ) {
    //     auto s = ::read(fd, &exp, sizeof(uint64_t));
    //     assertm(s == sizeof(uint64_t));

    //     tot_exp += exp;
    //     std::cout << "read: " << exp << ", total: " << tot_exp << std::endl;
    // }

    // exit(EXIT_SUCCESS);
}

void timer() {
    std::cout << "test timer" << std::endl;
    Eventloop loop;

    auto fun = []{
        std::cout << "event" << std::endl;
    };
    loop.addTimerNow(fun, 1000);

    loop.loop();
}

void threadpool() {
    TcpServer server("127.0.0.1", 1234);
}

int main() {
    threadpool();
    // timerfd();
    // timer();
    // loop_activate();
    // loop_time();
    // quit_activate();
    // threadloop_normal_use();
    // two_loop();
    // call_in_other_thread();

    return 0;
}
