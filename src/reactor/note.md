

对象状态管理
- 考虑方法调用的时序，比如start方法只能在第一次调用，应该有机制确保其他情况调用start方法的时候报错

impl模式
- 调试器不友好，不能直接看到impl类的细节

生命周期管理
- loop 生成，epoll fd创建
- 其他 fd 插入

连接的销毁
- conn持有channel，conn销毁时channel同样销毁，在channel执行过程中conn销毁


