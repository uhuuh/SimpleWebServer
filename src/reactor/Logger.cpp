#include "Logger.h"
#include <cstdarg>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <execinfo.h>
#include <cxxabi.h>
#include <cstdlib>
using namespace std;

// 解析符号信息并打印调用栈
void printStackTrace() {
    const int max_frames = 64;
    void *trace[max_frames];
    int frames = backtrace(trace, max_frames);
    char **symbols = backtrace_symbols(trace, frames);
    
    // 遍历每一帧并打印信息
    for (int i = 0; i < frames; ++i) {
        // 解析符号名
        char *parenthesis = strchr(symbols[i], '(');
        if (parenthesis == nullptr) {
            std::cerr << symbols[i] << std::endl;
            continue;
        }
        char *plus = strchr(parenthesis, '+');
        if (plus == nullptr) {
            std::cerr << symbols[i] << std::endl;
            continue;
        }
        *plus = '\0';
        
        int status;
        char *realname;
        realname = abi::__cxa_demangle(parenthesis + 1, nullptr, nullptr, &status);
        if (status != 0) {
            std::cerr << symbols[i] << std::endl;
            continue;
        }

        // 打印文件、函数和行号
        std::cout << symbols[i] << " ";
        std::cout << "(" << realname << "+";
        std::cout << std::string(plus + 1, strchr(plus, ')') - plus - 1) << ")" << std::endl;
        
        free(realname);
    }
    
    free(symbols);
}


void assertm(bool res) {
    if (!res) {
        printStackTrace();
        if (errno != 0) {
            throw std::runtime_error(strerror(errno));
        } else {
            throw std::runtime_error("");
        }
    }
}

void assertm(bool res, const char* error_msg) {
    if (!res) {
        throw std::runtime_error(error_msg);
    }
}

void check(bool res, const char* error_msg) {
    if (!res) {
        printf("%s\n", error_msg);
    }
}

void check_execption(std::function<void()> cb, const char* correct_msg, const char* error_msg) {
    bool is_correct = false;
    try {
        cb();
    } catch(...) {
        is_correct = true;
    }
    if (is_correct) {
        printf("%s\n", correct_msg);
    } else {
        printf("%s\n", error_msg);
    }
}

void FATAL(const string& msg) {
    cout << msg << endl;
    exit(-1);
}

void INFO(const string& msg) {
    // 带上调用栈，行名
    cout << msg << endl;
}


INITIALIZE_EASYLOGGINGPP
void easyloggingpp_init() {
    el::Configurations conf;
    conf.set(el::Level::Trace, el::ConfigurationType::Format, "[%datetime] [%levshort] [%thread_name] [%fbase:%line] - %msg");
    conf.set(el::Level::Info, el::ConfigurationType::Format, "[%datetime] [%levshort] [%thread_name] [%fbase:%line] - %msg");
    conf.set(el::Level::Fatal, el::ConfigurationType::Format, "[%datetime] [%levshort] [%thread_name] [%fbase:%line] - %msg");
    el::Loggers::reconfigureAllLoggers(conf);
    el::Loggers::setLoggingLevel(el::Level::Trace);
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
}

Logger Logger::logger;
Logger::Logger() {
    easyloggingpp_init();
}