#pragma once
#include "base.h"
#include <sys/epoll.h>


class Poller: noncopyable {
public:
    Poller();
    ~Poller();

    void addChannel(Channel* ch);
    void removeChannel(Channel* ch);
    void poll();
private:
    int epoll_fd;
    int activate_fd;
    int time_fd;
    set<int> fd_set;
    vector<epoll_event> event_list;
    static const int timeout_ms = 1000 * 3; 
};
