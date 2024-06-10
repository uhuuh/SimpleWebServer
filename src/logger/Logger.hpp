#pragma once
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iomanip>
#include <list>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <ctime>
#include <sys/select.h>
#include <thread>
#include <unistd.h>
#include <sstream>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
using namespace std;

class Logger {
public:
    enum class Level {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };
    vector<string> LevelString = {
        "TRACE",
        "DEBUG",
        "INFO ",
        "WARN ",
        "ERROR",
        "FATAL"
    };
    struct DateTime {
        tm t;
        timeval tv;
        DateTime() {
            gettimeofday(&tv, nullptr);
            // datetime = *localtime(&ti.tv_sec); 这是线程不安全的
            localtime_r(&tv.tv_sec, &t);
        }
        string get_date_str() {
            std::ostringstream oss;
            oss << put_time(&t, "%Y%m%d");
            return oss.str();
        }
        string get_time_str(bool with_us) {
            std::ostringstream oss;
            oss << put_time(&t, "%H%M%S");
            if (with_us) {
                oss << "." << tv.tv_usec;
            }
            return oss.str();
        }
    };
    struct DayTime {
        int day;
        int hour;
        int min;
        int sec;
        DayTime() {
            time_t ti = time(nullptr);
            tm t;
            localtime_r(&ti, &t);
            day = t.tm_mday;
            hour = t.tm_hour;
            min = t.tm_min;
            sec = t.tm_sec;
        }
        DayTime(int hour, int min, int sec): hour(hour), min(min), sec(sec) {
            time_t ti = time(nullptr);
            tm t;
            localtime_r(&ti, &t);
        }
        bool is_after(const DayTime& dt) {
            if (dt.day != day && dt.hour >= hour && dt.min >= min && dt.sec >= sec) return true;
            else return false;
        }
    };

    Level now_level = Level::TRACE;

    static Logger* get_instance() {
        static Logger logger;
        return &logger;
    }

    void append_log(Level level, const char* file, int line, const char* format, ...) {
        vector<char> temp(max_log_size);
        char *log = static_cast<char*>(&temp[0]);

        DateTime tp; // 使用系统时钟，可能会有时间回滚问题
        int writen = 0, add = 0;
        // todo 固定线程id的输出位数
        // todo snprintf是否加上最后0字符
        // todo snprintf 是否可以严格按照给定的缓冲区容量
        // todo 时间可能不是顺序递增

        add = snprintf(log, max_log_size, "%s %s %10lu %s | ", 
            tp.get_date_str().c_str(), tp.get_time_str(true).c_str(), pthread_self(), LevelString[static_cast<int>(level)].c_str());
        assert(add <= max_log_size - writen);
        writen += add;

        va_list args;
        va_start(args, format);
        add = vsnprintf(log + writen, max_log_size - writen, format, args);
        va_end(args);
        assert(add <= max_log_size - writen);
        writen += add;

        auto real_file = strchr(file, '/');
        if (real_file != nullptr) file = real_file + 1;
        add = snprintf(log + writen, max_log_size - writen, " | %s:%d\n", file, line);
        assert(add <= max_log_size - writen);
        writen += add;

        write_buf(log, writen);
    }


private:
    Logger(const string name="temp", bool is_async=true):
        name(name), is_async(is_async)
    {
        reset_file_fd();

        if (is_async) {
            is_thread_run = true;
            auto cb = bind(&Logger::flush_loop_in_thread, this);
            th = thread(cb);
        }
    }
    ~Logger() {
        if (is_async) {
            is_thread_run = false;
            conv.notify_one();
            th.join();
        }

        while (!buf_list.empty()) {
            flush_buf_with_lock();
        }

        close(fd);
    }
    struct BufBlock {
        int capacity;
        int writen;
        char* buf;
        BufBlock(int capacity): capacity(capacity), writen(0), buf(new char[capacity]) {}
        ~BufBlock() {
            delete [] buf;
        }
        int write_buf(const char *log, int size) {
            int n = min(capacity - writen, size);
            memcpy(buf, log, n);
            writen += n;
            return n;
        } 
        void flush_buf(int fd) {
            auto n = write(fd, buf, writen);
            assert(n == writen);
            writen = 0;
        }
        bool is_full() {
            return writen == capacity;
        }
    };
    void reset_file_fd() {
        if (fd != 0) close(fd);

        char file_name[max_log_size];
        DateTime tp;
        int n = sprintf(file_name, "%s-%s-%s-%d.log",
            name.c_str(), tp.get_date_str().c_str(), tp.get_time_str(false).c_str(), getpid());
        file_name[n] = 0; 

        fd = open(file_name, O_WRONLY | O_CREAT);
    }

    void write_buf(const char* log, int size) {
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
            flush_buf_with_lock();
        }
    }
    void flush_buf_with_lock() {
        // todo 只做了按照日志回滚，再做按照大小回滚
        // todo 添加参数控制flush一个块，还是全部块，刷新全部块应该使用递归，因为可能超过回滚大小

        DayTime ti;
        if (ti.is_after(roll_time)) {
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
            fprintf(stderr, "log too man, over has %d buffer block\n", a - buf_block_num);
        }

        if (!buf_list.empty()) {
            // printf("flush buf, size %d\n", buf_list.back()->writen);

            buf_list.front()->flush_buf(fd);
            empty_buf_list.push_back(buf_list.front());
            buf_list.pop_front();
        }
    }
    void flush_loop_in_thread() {
        unique_lock<mutex> lock(mu);
        auto cb = bind(&Logger::need_buf_flush, this);

        while (is_thread_run) {
            if (!need_buf_flush()) {
                conv.wait_for(lock, max_flush_ms, cb);
            }

            flush_buf_with_lock();
        }
    }

    string name;
    bool is_async;

    DayTime roll_time = {0, 0, 0};

    const int buf_block_num = 256;
    const int buf_block_size = 4 * 1024 * 1024;
    const chrono::milliseconds max_flush_ms = chrono::milliseconds(1000);
    const int max_log_size = 1024 * 2;
    inline bool need_buf_flush() {
        return buf_list.size() > 1;
    }

    int fd = 0;
    thread th;
    bool is_thread_run;
    mutex mu;
    condition_variable conv;
    list<shared_ptr<BufBlock>> buf_list;
    list<shared_ptr<BufBlock>> empty_buf_list;
};


#define LOG_TRACE(format, ...) if (Logger::Level::TRACE >= Logger::get_instance()->now_level) \
    {Logger::get_instance()->append_log(Logger::Level::TRACE, __FILE__ , __LINE__, format, ##__VA_ARGS__);}
#define LOG_DEBUG(format, ...) if (Logger::Level::DEBUG >= Logger::get_instance()->now_level) \
    {Logger::get_instance()->append_log(Logger::Level::DEBUG, __FILE__ , __LINE__, format, ##__VA_ARGS__);}
#define LOG_INFO(format, ...) if (Logger::Level::INFO >= Logger::get_instance()->now_level) \
    {Logger::get_instance()->append_log(Logger::Level::INFO, __FILE__ , __LINE__, format, ##__VA_ARGS__);}
#define LOG_WARN(format, ...) if (Logger::Level::WARN >= Logger::get_instance()->now_level) \
    {Logger::get_instance()->append_log(Logger::Level::WARN, __FILE__ , __LINE__, format, ##__VA_ARGS__);}
#define LOG_ERROR(format, ...) if (Logger::Level::ERROR >= Logger::get_instance()->now_level) \
    {Logger::get_instance()->append_log(Logger::Level::ERROR, __FILE__ , __LINE__, format, ##__VA_ARGS__);}
#define LOG_FATAL(format, ...) if (Logger::Level::FATAL >= Logger::get_instance()->now_level) \
    {Logger::get_instance()->append_log(Logger::Level::FATAL, __FILE__ , __LINE__, format, ##__VA_ARGS__);}
// todo 确保fatal退出时所有日志刷新到磁盘上
