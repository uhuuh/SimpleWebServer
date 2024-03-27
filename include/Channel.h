#pragma once
#include "base.h"

enum class EventType  {
    READ = 1,
    WRITE = 2
};

class Channel: noncopyable {
public:
    Channel(Eventloop*, int);
    ~Channel();

    void enableEvent(EventType, Callback);
    void disableEvent(EventType);
    void handleEvent();

    bool activateRead = false;
    bool activateWrite = false;
    bool enableRead = false;
    bool enableWrite = false;
    const fd_t fd = -1;
private:
    Eventloop *loop = nullptr;
    Callback handle_read_cb;
    Callback handle_write_cb;
};



