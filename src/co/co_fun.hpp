#include <cstddef>
#include <cstdio>
#include <unistd.h>


class CorouteScheduler;
void co_env_init(CorouteScheduler* co_sche);

ssize_t co_read(int fd, void* buf, size_t size);

ssize_t co_write(int fd, const void* buf, size_t size);

class sockaddr;
ssize_t co_accept(int fd, struct sockaddr* addr, socklen_t* len);

void co_sleep(int ms);

void co_close(int fd);

