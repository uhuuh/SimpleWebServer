#include "Poller.h"
#include "Channel.h"


void Poller::updateChannel(Channel* channel) {
    auto fd = channel->getFd();
    if (channel->isEnableEvent(EventType::READ)) {
        FD_SET(fd, &m_readFds);
    } 
    if (channel->isEnableEvent(EventType::WRITE)) {
        FD_SET(fd, &m_writeFds);
    } 
    if (channel->isEnableEvent(EventType::EXCEPTION)) {
        FD_SET(fd, &m_exceptionFds);
    }
    m_channelMap[fd] = channel;
    m_maxFd = std::max(fd, m_maxFd);
}

void Poller::removeChannel(fd_t fd) {
    assertm(m_channelMap.find(fd) != m_channelMap.end());

    FD_CLR(fd, &m_readFds);
    FD_CLR(fd, &m_writeFds);
    FD_CLR(fd, &m_exceptionFds);
    m_channelMap.erase(fd);
    m_maxFd = m_channelMap.empty() ? 0 : m_channelMap.rbegin()->first;
}

Poller::Poller():
    m_maxFd(0),
    m_channelMap()
{
    FD_ZERO(&m_readFds);
    FD_ZERO(&m_writeFds);
    FD_ZERO(&m_exceptionFds);
}

void Poller::poll(uint32_t timeoutSec, std::vector<Channel*>* channels) {
    auto read_fds_copy = m_readFds;
    auto write_fds_copy = m_writeFds;
    auto exception_fds_copy = m_exceptionFds;
    struct timeval timeout{timeoutSec, 0};

    auto n_fd = ::select(m_maxFd + 1, &read_fds_copy, &write_fds_copy, &exception_fds_copy, &timeout);
    assertm(n_fd >= 0);

    for (fd_t fd = 0; fd < m_maxFd + 1; ++fd) {
        auto ptr = m_channelMap.find(fd);
        if (ptr == m_channelMap.end()) {
            continue;
        }

        bool flag_activate = false;
        if (FD_ISSET(fd, &read_fds_copy) && ptr->second->isEnableEvent(EventType::READ)) {
            flag_activate = true;
            ptr->second->activateEvent(EventType::READ);
        }
        if (FD_ISSET(fd, &write_fds_copy) && ptr->second->isEnableEvent(EventType::WRITE)) {
            flag_activate = true;
            ptr->second->activateEvent(EventType::WRITE);
        }
        if (FD_ISSET(fd, &exception_fds_copy) && ptr->second->isEnableEvent(EventType::EXCEPTION)){
            flag_activate = true;
            ptr->second->activateEvent(EventType::EXCEPTION);
        }

        if (flag_activate) {
            channels->push_back(ptr->second);
        }
    }
}
