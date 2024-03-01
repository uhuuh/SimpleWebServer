#pragma once

#include "type.h"


class Poller: noncopyable {
public:
    Poller();
    ~Poller() = default;

    void updateChannel(Channel* channel);
    void removeChannel(fd_t fd);
    void poll(uint32_t timeoutMs, std::vector<Channel*>* channels);
    fd_t m_maxFd;
    fd_set m_readFds, m_writeFds, m_exceptionFds;
    std::map<fd_t, Channel*> m_channelMap;
};