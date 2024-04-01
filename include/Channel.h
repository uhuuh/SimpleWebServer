#pragma once
#include "EventLoop.h"
#include "base.h"

enum class EventType  {
    READ = 1,
    WRITE = 2
};

class Channel: noncopyable {
public:
    Channel(Eventloop*, fd_t);
    ~Channel();

    void addEvent(EventType, Callback);
    void enableEvent(EventType);
    void disableEvent(EventType);
    void handleEvent();

    bool activateRead = false;
    bool activateWrite = false;
    bool enableRead = false;
    bool enableWrite = false;
    const fd_t fd = -1;
protected:
    Eventloop * const loop = nullptr;
private:
    Callback handle_read_cb;
    Callback handle_write_cb;
};



