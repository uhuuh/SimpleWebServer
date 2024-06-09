

## logger backend
- 块个数 buf_block_size
- 块大小 buf_block_num
- 最大flush时间 max_flush_ms
- 同步还是异步 is_async
- roll_time_point

- now_buf_writed
- is_flush_thread_run
- roll_day
- 当前 level

- name （日志名-日期-时间-进程id.log)
- level: trace debug info warn error fatal
- 格式：日期 时间.微秒 线程 级别（占5个字符） 正文-源文件名:行号（末尾添加换行符）

init
- if 是异步模式，启动flush线程
- 日志名字 name-日期-时间.log

set_level

append(char* format, ...) 
- 使用本地char数组 构造完str
- output

write_buf(char* str, int len)
- 加互斥锁
- remain = len
- while remain != 0
    - 如果buf list 为空，从 free buf list 

    - 如果buf队列为空，
        - 从full_buf中剔除第一个添加到buf队列中，buf_writen = 0
    - 计算空余，buf_writen, remain
    - 如果buf写满，加入到buf_full队列中

- 如果是同步模式的话，flush

flush_loop
- temp——buf  list
- while is_flush_thread_run
    - 加互斥锁
    - if full_buf为空,conv wait_for max_flush_ms
    - swap temp_buf list and buf list
    - 互斥锁解锁
    - flush temp_buf_list中每个
    - temp_buf_list clear

flush_buf
- 如果当前时间超过回滚时间 且 今天不等于 roll day，关闭日志文件，重新创建一个日志文件
- 对于 buf中所有 fflush
 
destroy
- 调用一下flush
- is_flush_thread_run false
- flush thread flush


## note
- POSIX IO与标准 IO
- 单例模式 析构函数调用问题


