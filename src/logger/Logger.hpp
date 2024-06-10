#pragma once
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdarg>
#include <cstdio>
#include <cstring>
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
            oss << put_time(&t, "%Y-%m-%d");
            return oss.str();
        }
        string get_time_str(bool with_us) {
            std::ostringstream oss;
            oss << put_time(&t, "%H:%M:%S");
            if (with_us) {
                oss << '.';
                if (tv.tv_usec < 100000) oss << '0';
                oss << tv.tv_usec;
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
            day = t.tm_mday;
        }
        bool is_after(const DayTime& dt) {
            if (dt.day != day && dt.hour >= hour && dt.min >= min && dt.sec >= sec) return true;
            // if (dt.hour >= hour && dt.min >= min && dt.sec >= sec) return true;
            else return false;
        }
    };

    Level now_level = Level::TRACE;

    static Logger* get_instance() {
        static Logger logger;
        return &logger;
    }

    void append_log(Level level, const char* file, int line, const char* format, ...);

private:
    Logger();
    ~Logger();
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
            memcpy(buf + writen, log, n);
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
    void reset_file_fd();
    void write_buf(const char* log, int size);
    void flush_buf(bool is_full=false);
    void flush_loop_in_thread();

    string name = "temp";
    bool is_async = true;

    DayTime roll_time = {0, 0, 0};
    const int roll_file_size = 8 * 1024 * 1024;

    const int buf_block_num = 256;
    const int buf_block_size = 4 * 1024 * 1024;
    const chrono::milliseconds max_flush_ms = chrono::milliseconds(1000);
    const int max_log_size = 1024 * 2;
    inline bool need_buf_flush() {
        return buf_list.size() > 1;
    }

    int fd = 0;
    thread th;
    bool is_print = false;
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

