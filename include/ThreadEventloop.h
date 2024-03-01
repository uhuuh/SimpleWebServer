#pragma once
#include "type.h"

class ThreadEventloop: noncopyable {
public:
    ThreadEventloop() = default;
    ~ThreadEventloop();

    Eventloop* getLoop();
    void _runLoop();
    bool m_flagHasLoop = false;
    std::unique_ptr<std::thread> m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    Eventloop* m_loop = nullptr;
};
