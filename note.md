

- 只有编译万可执行的文件, 在IDE的行号上才有执行三角按钮
- xmake project -k cmake 根据xmake配置生成对应的cmake配置
- 问题
  - 尽管使用了头文件避免重复包含机制, 还是有重复声明的链接错误
  - accept和select返回-1
- 要点
    - 不要依赖初始化列表中的初始化顺序. 成员变量的初始化顺序是由它们在类中声明的顺序决定的，而不是由初始化列表中的顺序决定的, 这隐含了初始化成员列表的顺序应该和类中声明的顺序一样
    - 如果成员变量有const, 默认构造函数会被删除. const成员变量不能通过拷贝初始化
    - return数组或者tuple可以拆包
    - 类的bool成员不会默认初始化为false
    - https://stackoverflow.com/questions/28236870/error-undefined-reference-to-stdcout 使用g++编译cpp文件, 因为可以默认链接到cpp标准库
    - Socket不可复制, 由于函数传递过程中导致Socket创建又销毁, 销毁时释放掉占用的资源, 新的Socket中的fd实际上是无效的
    - string_view本省不可修改, 不能从string_view转换到const string_view
- todo
  - logger
  - 多线程
  - 连接状态管理
  - 
## 类
Buffer
- pushFromSocket(fd) buffer空间不够时需要扩容
- popToSocket(fd) buffer没有全部写完的时候, 需要获取buffer中还剩多少内容
- getContentSize()

## muduo
EvventLoopThread 在thread内执行一个loop, 并且将其暴露给外部
- void threadFunc() 
  - 函数内构造一个loop
  - 获取锁
  - 将内部的loop赋值为外部的loop_
  - 条件变量通知, 释放锁
  - loop.loop
- Thread thread_ 初始化时传入threadFunc
- EventLoop loop_
- MutexLock mutex_
- Condition cond_
- void startLoop()
  - 执行thread中的函数, 返回loop_
  - 获取锁
  - 条件变量等待直到loop_被赋值完成
  - 释放锁
  - return loop_

EventLoopThreadPool
- EventLoop* baseLoop_ 由构造函数传入一个baseloop来初始化
- vector<EventLoopThread> threads_
- vector<EventLoop*> loops_
- void start()
  - 按照给定的numThreads, 初始化这么多个EventLoopThread, 将他们填入threads_
  - 并且每个执行startLoop获取内部loop填入loops_中
- EventLoop* getNextLoop()
  - 将要返回的loop初始化为baseLoop_
  - 如果loops_中不空的话, 将loop初始化为next_一个
  - return loop

ThreadPool
  - void start(int)
    - threads_初始化为传入数字的大小
    - 往threads_传入同样大小个Thread, 使用runInThread回调
  - void runInThread()
    - 跑threadInitCallback_回调
    - 调用take从queue_中获取Task
    - 执行Task
  - void stop()
    - 所有条件变量通知
    - 所有Thread join, 销毁线程
  - void run(Task)
    - 往queu_传入Task
  - `std::vector<std::unique_ptr<muduo::Thread>> threads_`
  - `std::deque<Task> queue_ GUARDED_BY(mutex_)`

多线程Tcpserver
- 内部持有一个eventloop线程池和baseEventLoop
- baseEventLoop负责监听连接请求
- 监听到连接请求时从eventloop线程池中循环获取一个Eventloop, 将建立连接回调传入EventLoop的FunctorList中(有互斥锁)
- 上次获取的Eventloop在loop中, poll之后执行FunctorList中的函数, 执行建立连接回调函数, 将channel添加进该Eventloop的poller中

至于用户计算过程也使用线程池, 也是用户在Tcpserver中维护一个Tcpserver, 然后在onMessage将计算任务添加进该线程池中