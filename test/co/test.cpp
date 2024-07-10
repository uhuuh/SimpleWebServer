#include "Coroutine.hpp"
#include "Eventloop.hpp"
#include "Logger.hpp"
#include "base.hpp"
#include "co_fun.hpp"
#include <cmath>
#include <cstring>
#include <unistd.h>

bool isNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Failed to get file descriptor flags: " << strerror(errno) << std::endl;
        return false;
    }
    return (flags & O_NONBLOCK) != 0;
}

void setNonBlocking(int fd) {
    // 获取文件描述符的当前标志
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Failed to get file descriptor flags: " << strerror(errno) << std::endl;
        return;
    }

    // 将文件描述符设置为非阻塞模式
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        std::cerr << "Failed to set file descriptor to non-blocking: " << strerror(errno) << std::endl;
        return;
    }
}

int main() {
    Logger::get_instance()->is_print = true;

    EventLoop loop;
    CoroutinePool co_pool;
    CorouteScheduler sche(&loop, &co_pool);

    int fds[4];
    assertm(pipe(fds) >= 0);
    assertm(pipe(fds + 2) >= 0);
    for (int i = 0; i < 4; ++i) {
        setNonBlocking(fds[i]);
    }

    int count = 2;

    auto cb1 = [&] {
        const char* me = "message 1";
        char buf[1024];

        co_write(fds[1], (void*)me, strlen(me));

        co_read(fds[2], (void*)buf, sizeof(buf));
        cout << buf << endl;

        count -= 1;
    };
    auto cb2 = [&] {
        const char* me = "mesage 2";
        char buf[1024];

        co_read(fds[0], buf, sizeof(buf));
        cout << buf << endl;

        co_write(fds[3], (void*)me, strlen(me));

        count -= 1;
    };
    auto cb3 = [&] {
        while (count != 0) {
            co_sleep(10);
            LOG_DEBUG("sleep timer cb");
        }

        loop.stop();
    };
    sche.add_callback(cb1);
    sche.add_callback(cb2);
    sche.add_callback(cb3);

    loop.run();

    LOG_ERROR("test finish");
    return 0;
}
