
## 介绍

实现一个Linux环境上基于Reactor模式的TCP并发服务器

模块
- base 一些公共的函数和类型定义
- logger 日志模块，支持同步模式和异步模式，异步模式下硬盘写入速度200MB/s以上
- reactor 事件监听模块，支持fd上的读写事件和时间事件，事件触发后执行事件上绑定的回调
- tcp TCP服务器和客户端，服务器能完成多个TCP连接上的请求处理，客户端支持建立连接失败时的超时重连
- http HTTP模块 http解析器完成，http服务器待完成
- co 协程模块，待完成
- rpc RPC模块，待完成
- 压力测试 待完成

## 执行

项目文件组成
- src目录是源代码目录，其下面的子目录对应各模块的源代码
- test目录是测试目录，其下面的子目录对应各模块的测试代码

在linux系统下执行如下命令可运行各模块测试代码
```
mkdir build
cd build
cmake ..
make
<执行各模块的测试文件>
```

## logger模块

使用LOG_XXXXX宏来打印日志，使用局部静态变量方式实现的懒汉单例模式初始化全局单一Logger实例

Logger内部维护两个缓冲块列表buf_list和empty_buf_list

同步模式下，Logger依次调用如下方法
- append_log 无锁，日志进行格式化并将其保存在堆对象上的局部缓冲区，这个方法结束时释放堆对象的空间
- write_buf 有锁，日志从局部缓冲区复制到缓冲块中。日志写入buf_list的末尾块中，如果buf_list为空，从empty_buf_list拿取一个块，如果empty_buf_list为空，则新创建一个缓冲块加入buf_list。
- flush_buf 有锁，日志从缓冲块刷盘到硬盘文件中。首先清除多余内存块，如果buf_list和empty_buf_list两个列表上块的大小超过buf_block_num，依次丢掉empty_buf_list和buf_list末尾的块使得块总数不超过buf_block_num，这避免了频繁写日志占满内存。然后从buf_list拿取头部缓冲块，将这个缓冲块的内容直接IO刷盘，再将这个缓冲块放入empty_buf_list的末尾

异步模式下，Logger调用append_log和write_log两个方法将日志下入到缓冲块中，另外开启一个线程执行循环，在循环中判断有缓冲块慢或者等待时间超过max_flush_ms，则执行flush_buf操作。在Logger析构时，终止线程函数，同时将buf_list上的所有缓冲块刷盘

## reactor模块

EventLoop对象执行时间循环，支持注册和修改fd上的读写事件，添加和修改时间事件。EventLoop的事件循环会阻塞程序执行，一般放入线程中，一个线程与一个EventLoop绑定。线程池多个线程中有多个EventLoop，充分立刻机器CPU多核增加事件处理能力
- Poller提供多路复用IO操作的封装，支持fd上读写事件修改和取消
- Channel提供操作fd读写事件的方式，**其销毁时从poll而取消监听fd，并关闭fd**
- TimerQueue内部使用最小堆维护一个定时器队列, time fd设定通知时间，EventLoop添加time fd和读时间上回调（处理激活的定时器）
    - 定时器id内部有激活时间、定时器序列号、定时器指针，结合激活时间和序列号两个字段可以判断该定时器是否被激活
    - 使用time fd提供时间通知功能，time fd设定最近激活的定时器时间，当fd可读时遍历定时器队列激活所有超时的定时器
- 回调列表 **对象的各种回调注册到EventLoop中来完成实际功能，在多线程EventLoop中，一个对象拥有fd并且一个EventLoop绑定，对象上有fd读写事件的回调，也有与其他对象交互的回调，后一种回调可能在其他线程执行，后一种回调有线程安全问题。**每个EventLoop维护一个回调列表。如果一个对象的回调尽在EventLoop内部执行，在执行断言当前线程必须是绑定EventLoop所在线程，如果一个回调可能在外部执行，将该回调添加到EventLoop的回调列表中。EventLoop一次多路复用IO的等待后，先执行激活fd上的所有事件回调，然后执行回调列表上的所有事件
- Activater激活阻塞的多路复用IO，使回调列表中的回调可以立刻执行

## tcp模块

TCPServer 
- 连接表 连接fd到连接对象的映射表
- 创建连接回调 创建一个连接，从线程池EventLoop中获取一个EventLoop绑定连接，连接保存到连接表。如果连接数量已达到最大连接数量，则关闭这个连接
- 销毁连接回调 连接表中销毁连接
- 主EventLoop 阻塞程序运行防止程序退出，一般处理accept fd和连接连接销毁回调
- 线程池EventLoop 一般处理peer fd
- 用户回调组 用户提供如下回调来实现应用层功能，比如message中对消息分包和响应消息来提供http功能
    - open 打开连接时调用
    - close 关闭连接时调用
    - message 连接接受消息时调用
    - write_high 读缓存区内容积压太多时调用
    - write_low 写缓冲区清空时调用
- TCPAcceptor listen fd添加到EventLoop中，当该fd可读时调用创建连接回调
- TCPConnection 进行连接的状态管理、连接数据读写到应用层缓冲中、依据连接的各种状态调用用户回调
    - 应用层缓冲 对于读操作来说，TCP面向字节流服务，在消息为完整之间需要将消息保存到应用层缓冲中，另外fd 水平触发模型下必须将fd的数据读完避免该fd频繁触发。对于写操作，也需要引用层缓冲保存未发送完的数据
    - 状态有connecting、half_close（写端关闭）、full_close三种。**注意在half_close后，如果写缓冲中还有数据，需要等待写缓冲数据发送完才半关闭连接**
    - 连接销毁，在自己的回调中判断需要销毁时，调用外部提供的销毁连接回调，将该回调添加绑定EventLoop对应回调列表中，避免自己销毁自己后再访问对象成员变量出错（**实际上直接调用销毁连接回调，这种情况有发生，销毁连接时也销毁channel，自己的回调实际上是channel内保存的读写回调，在这些回调结束后还会对channel进行修改，但这是channel已经随着连接对象销毁了**）

TCPClient
- 用户回调组
- 连接
- 主EventLoop 阻塞程序运行防止程序退出，处理peer fd
- TCPConnector connect fd首先以阻塞方式初始化，连接连接失败后在EventLoop添加事件事件来延迟重试，连接连接成功后fd改为非阻塞方式加入EventLoop中



