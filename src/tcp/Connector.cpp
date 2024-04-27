#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include "base.h"
#include <arpa/inet.h>


Connector::Connector(
    Eventloop* loop, 
    CreateConnectionCallback create_conn_cb,
    const std::string& ip,
    in_port_t port
):
    Channel(loop, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)),
    create_conn_cb(create_conn_cb),
    ip(ip),
    port(port)
{
    // Channel析构的时候会从loop中取消注册，故loop中必须添加channel的fd
    // auto cb = bind(&Connector::connect_peer, this);
    // addEvent(EventType::READ, cb);
    // disableEvent(EventType::READ);

    INFO(format("connect_create | fd: {}", fd));
    loop->addCallback([this](){this->connect_peer();});
}


void Connector::connect_peer() {
    assertm(loop->isInSameThread());

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    // todo connect 是否可以使用poll监听
    // todo connect 非阻塞问题 
    auto ret = connect(fd,  (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        auto cb = [this]() {
            this->connect_peer();
        };

        if (retry_delay_ms <= max_retry_delay_ms) {
            INFO(format("connect_retry | after_ms: {}", retry_delay_ms));
            loop->addTimer(cb, retry_delay_ms);
            retry_delay_ms *= 2;
        } else {
            throw;
        }
    } else {
        INFO(format("connect_succ | after_ms: {}", retry_delay_ms));
        create_conn_cb(fd, ip, port);
    }
}

