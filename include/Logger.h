#pragma once
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <functional>

void assertm(bool res);
void assertm(bool res, const char* error_msg);

void check_execption(std::function<void()> cb, const char* correct_msg, const char* error_msg);
void check(bool res, const char* error_msg);

void FATAL(const char* msg);
void INFO(const char* msg);
void DEBUG(const char* msg);

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

