#pragma once
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <functional>
#include <sstream>
using namespace std;

void assertm(bool res);
void assertm(bool res, const char* error_msg);

void check_execption(std::function<void()> cb, const char* correct_msg, const char* error_msg);
void check(bool res, const char* error_msg);

void INFO(const string& msg);

template<typename Container>
std::string join(const Container& cont, const std::string& delim) {
    std::ostringstream oss;
    if (cont.size() == 0) return "";

    auto it = std::begin(cont);
    oss << *it;
    ++it;
    for ( ; it != std::end(cont); ++it) {
        oss << delim;
        oss << *it;
    }
    return oss.str();
}

template<typename T>
string to_str(const T& obj) {
    std::ostringstream oss;
    oss << obj;
    return oss.str();
}

template<typename... Args>
void format_help(vector<string>& buf) {

}

template<typename T, typename... Args>
void format_help(vector<string>& buf, T arg, Args... args) {
    buf.push_back(to_str(arg));
    format_help(buf, args...);
}

template<typename... Args>
string format(const string& format, Args&&... args) {
    vector<string> buf_arg;
    format_help(buf_arg, args...);

    ostringstream oss;
    vector<string> format_buf;
    auto start_format = 0;
    auto start_arg = 0;
    while (true) {
        auto pos = format.find("{}", start_format);
        if (pos != string::npos) {
            oss << format.substr(start_format, pos - start_format);
            oss << buf_arg[start_arg];
            start_format = pos + 2;
            start_arg += 1;
        } else {
            oss << format.substr(start_format);
            assertm(start_arg == buf_arg.size());
            break;
        }
    }

    return oss.str();
}

#include "easyloggingpp.h"
#define LOG_TRACE LOG(TRACE)
#define LOG_INFO LOG(INFO)
#define LOG_FATAL LOG(FATAL)

class Logger {
private:
	static Logger logger;
private:
	Logger();
	~Logger() = default;
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
public:
	static Logger& getLogger() {
		return logger;
	}
};

