#include <bits/types/timer_t.h>
#include <iostream>
#include <chrono>
#include <ctime>

// 将时间转换为人类可读的时间字符串形式
std::string formatTime(time_t time) {
    // 将 time_t 转换为 tm 结构
    std::tm tm = *std::localtime(&time);

    // 使用 std::put_time 格式化时间字符串
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return buffer;
}

int main() {
    // 获取系统单调时钟时间
    auto monotonicTime = std::chrono::steady_clock::now();
    // 获取系统实时时钟时间
    auto realtimeTime = std::chrono::system_clock::now();

    // 打印系统单调时钟时间
    std::cout << "System monotonic clock time: " << formatTime( std::chrono::system_clock::to_time_t(monotonicTime)) << std::endl;
    // 打印系统实时时钟时间
    std::cout << "System realtime clock time: " << formatTime(std::chrono::system_clock::to_time_t(realtimeTime)) << std::endl;

    return 0;
}
