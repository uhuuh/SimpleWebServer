#pragma once
#include "type.h"

enum class EventType  {
    READ = 1,
    WRITE = 2,
    EXCEPTION = 3
};

class Channel: noncopyable {
public:
    Channel() = delete;
    explicit Channel(fd_t fd);
    ~Channel() = default;

    void addEvent(EventType event, Callback callback);
    void removeEvent(EventType event);
    bool isEnableEvent(EventType event) const;
    void activateEvent(EventType event);
    void handleEvent();
    fd_t getFd();

    fd_t m_fd;
    bool m_enableRead;
    bool m_enableWrite; 
    bool m_enableException;
    bool m_activateRead;
    bool m_activateWrite;
    bool m_activateException;
    Callback m_readCallback;
    Callback m_writeCallback;
    Callback m_exceptionCallback;
};

