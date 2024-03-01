#include "type.h"
#include "ThreadEventloop.h"
#include "Eventloop.h"
#include <iostream>

void two_loop() {
    ThreadEventloop tloop;
    tloop.getLoop();

    check_execption([&]{tloop.getLoop();}, "correct", "error");
}

void diff_thread() {
    ThreadEventloop tloop;
    auto loop = tloop.getLoop();

    check_execption([&]{loop->assertSameThread();}, "correct", "error");
}

void diff_two_thread() {
    ThreadEventloop tloop1;
    ThreadEventloop tloop2;
    auto loop1 = tloop1.getLoop();
    auto loop2 = tloop2.getLoop();
    assertm(loop1->m_threadId != loop2->m_threadId);
}

int main() {
    std::cout << "test ThreadEventloop" << std::endl;
    diff_two_thread();
    two_loop();
    diff_thread();
    return 0;
}