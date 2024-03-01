#include "Channel.h"

Channel::Channel(fd_t fd):
    m_fd(fd),
    m_enableRead(false),
    m_enableWrite(false),
    m_enableException(false),
    m_activateRead(false),
    m_activateWrite(false),
    m_activateException(false)
{}

void Channel::addEvent(EventType event, Callback callback) {
    if (event == EventType::READ) {
        this->m_enableRead = true;
        this->m_readCallback = callback;
    } else if (event == EventType::WRITE) {
        this->m_enableWrite = true;
        this->m_writeCallback = callback;
    } else if (event == EventType::EXCEPTION) {
        this->m_enableException = true;
        this->m_exceptionCallback = callback;
    } else {
        FATAL("event type error");
    }
    // LOG_TRACE << "addEvent " << int(event);
}

void Channel::removeEvent(EventType event) {
    if (event == EventType::READ) {
        this->m_enableRead = false;
    } else if (event == EventType::WRITE) {
        this->m_enableWrite = false;
    } else if (event == EventType::EXCEPTION) {
        this->m_enableException = false;
    } else {
        FATAL("event type error");
    }
}

void Channel::activateEvent(EventType event) {
    if (event == EventType::READ) {
        assertm(m_enableRead);
        m_activateRead = true;
    } else if (event == EventType::WRITE) {
        assertm(m_enableWrite);
        m_activateWrite = true;
    } else if (event == EventType::EXCEPTION) {
        assertm(m_enableException); 
        m_activateException = true;
    } else {
        FATAL("event type error");
    }

}

bool Channel::isEnableEvent(EventType event) const {
    if (event == EventType::READ) {
        return this->m_enableRead;
    } else if (event == EventType::WRITE) {
        return this->m_enableWrite;
    } else if (event == EventType::EXCEPTION) {
        return this->m_enableException;
    } else {
        FATAL("event type error");
        return false;
    }
}

void Channel::handleEvent() {
    if (m_activateRead) {
        m_readCallback();
    }
    if (m_activateWrite) {
        m_writeCallback();
    }
    if (m_activateException) {
        m_exceptionCallback();
    }
    m_activateRead = false;
    m_activateWrite = false;
    m_activateException = false;
}

fd_t Channel::getFd() {
    return m_fd;
}

