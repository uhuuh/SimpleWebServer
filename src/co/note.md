

库打桩机制 https://hansimov.gitbook.io/csapp/part2/ch07-linking/7.13-library-interpositioning

malloc包装版本中段错误
- 里面使用printf，printf会调用malloc，使得无穷递归爆栈而段错误
- 可以使用sprintf打印到err，或者使用fputs
- https://stackoverflow.com/questions/75554316/segmentation-fault-when-making-a-runtime-stub-in-malloc-using-printf

资源管理
- 固定的资源 放置在固定位置，使用指针操作该资源
- 动态销毁的资源 定义一个对象管理该资源，使用智能指针操作该资源

