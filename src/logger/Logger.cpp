#include "Logger.hpp"

#include <exception>
#include <functional>
#include <stdexcept>

Logger::Logger() {
    reset_file_fd();

    if (is_async) {
        is_thread_run = true;
        auto cb = std::bind(&Logger::flush_loop_in_thread, this);
        th = thread(cb);
    }
}

Logger::~Logger() {
    if (is_async) {
        is_thread_run = false;
        conv.notify_one();
        th.join();
    }

    flush_buf(true);

    close(fd);
}

void Logger::append_log(Level level, const char *file, int line,
                        const char *format, ...) {
    vector<char> temp(max_log_size);
    char *log = static_cast<char *>(&temp[0]);

    DateTime tp; // 使用系统时钟，可能会有时间回滚问题
    int writen = 0, add = 0;
    // todo 固定线程id的输出位数
    // todo snprintf 是否可以严格按照给定的缓冲区容量
    // todo 时间可能不是顺序递增

    // sprintf 按format写入字符串到buf中，字符串末尾写入0，返回的数字非负值是字符串的长度（不含末尾0）
    add = snprintf(log, max_log_size, "%s %s %10lu %s | ",
        tp.get_date_str().c_str(), 
        tp.get_time_str(true).c_str(),
        pthread_self(), 
        LevelString[static_cast<int>(level)].c_str()
    );
    assert(add + 1 <= max_log_size - writen);
    writen += add;

    va_list args;
    va_start(args, format);
    add = vsnprintf(log + writen, max_log_size - writen, format, args);
    va_end(args);
    assert(add + 1 <= max_log_size - writen);
    writen += add;

    auto real_file = strchr(file, '/');
    if (real_file != nullptr)
        file = real_file + 1;
    add = snprintf(log + writen, max_log_size - writen, " | %s:%d\n", file, line);
    assert(add + 1 <= max_log_size - writen);
    writen += add;

    if (is_print) printf("%s", log);

    write_buf(log, writen);

    if (level >= Level::FATAL) {
        flush_buf(true);
        throw runtime_error("fatal log");
    }
}

void Logger::reset_file_fd() {
    if (fd != 0)
        close(fd);

    char file_name[max_log_size];
    DateTime tp;
    int n = sprintf(file_name, "%s_%s_%s_%d.log", name.c_str(),
                    tp.get_date_str().c_str(), tp.get_time_str(false).c_str(),
                    getpid());
    file_name[n] = 0;

    fd = open(file_name, O_WRONLY | O_CREAT);
}

void Logger::write_buf(const char *log, int size) {
    lock_guard<mutex> lock(mu);

    int remain = size;
    while (remain > 0) {
        if (buf_list.empty() || buf_list.back()->is_full()) {
            if (empty_buf_list.empty()) {
                buf_list.push_back(make_shared<BufBlock>(buf_block_size));
            } else {
                buf_list.push_back(empty_buf_list.front());
                empty_buf_list.pop_front();
            }
        }

        int n = buf_list.back()->write_buf(log, size);
        remain -= n;
    }

    if (is_async) {
        if (need_buf_flush()) {
            conv.notify_one(); // todo 每次插入日志都notify会很耗时吗
        }
    } else {
        flush_buf();
    }
}

void Logger::flush_buf(bool is_full) {
    // todo 只做了按照日志回滚，再做按照大小回滚
    // todo
    // 添加参数控制flush一个块，还是全部块，刷新全部块应该使用递归，因为可能超过回滚大小

    DayTime ti;
    if (ti.is_after(roll_time)) {
        printf("log roll\n");
        roll_time = ti;
        reset_file_fd();
    }

    int a = buf_list.size() + empty_buf_list.size();
    if (a > buf_block_num) {
        int b = min(static_cast<int>(empty_buf_list.size()), a - buf_block_num);
        empty_buf_list.resize(b);
        if (buf_list.size() > buf_block_num) {
            buf_list.resize(buf_list.size() - buf_block_num);
        }
        fprintf(stderr, "log too man, over has %d buffer block\n",
                a - buf_block_num);
    }

    if (!buf_list.empty()) {
        // printf("flush buf, size %d\n", buf_list.back()->writen);

        buf_list.front()->flush_buf(fd);
        empty_buf_list.push_back(buf_list.front());
        buf_list.pop_front();
    }

    if (!buf_list.empty() && is_full)
        flush_buf(is_full);
}

void Logger::flush_loop_in_thread() {
    unique_lock<mutex> lock(mu);
    auto cb = std::bind(&Logger::need_buf_flush, this);

    while (is_thread_run) {
        if (!need_buf_flush()) {
            conv.wait_for(lock, max_flush_ms, cb);
        }

        flush_buf();
    }
}
