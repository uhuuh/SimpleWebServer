#include "type.h"

int main(int argc, char* argv[]) {
   #ifdef ELPP_THREAD_SAFE
      LOG(DEBUG) << "easylogging++ | 已定义 ELPP_THREAD_SAFE，easylogging++ 线程安全！";
   #else
      LOG(ERROR) << "easylogging++ | 未定义 ELPP_THREAD_SAFE，easylogging++ 线程不安全！程序可能崩溃，请定义 ELPP_THREAD_SAFE！";
   #endif

   auto f1 = [] {
      LOG_INFO << "My first info log using default logger" << std::this_thread::get_id();
   };
   auto f2 = [] {
      LOG(INFO) << "abc" << std::this_thread::get_id();
   };
   std::thread t1(f1);
   std::thread t2(f2);
   t1.join();
   t2.join();

   LOG(INFO) << "fdsa";
   LOG(INFO) << "abc";

   return 0;
}