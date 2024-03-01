#include "Logger.h"
#include <cstdarg>
#include <sstream>

void assertm(bool res) {
    if (!res) {
        if (errno != 0) {
            // printf("assert error: %s\n", strerror(errno));
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

void FATAL(const char* msg) {
    printf("%s\n", msg);
    exit(-1);
}

void INFO(const char* msg) {
    printf("%s\n", msg);
}

void DEBUG(const char* msg) {
    printf("%s\n", msg);
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