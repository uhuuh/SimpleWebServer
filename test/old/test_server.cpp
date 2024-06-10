#include "base.h"
#include "TcpServer.h" 
// todo 尝试解决unique_ptr不完整类型


int main() {
    ifstream file("/mnt/e/MY_STUDY/subject/network_study/study_muduo/test/test.txt"); // 打开文件
    if (!file.is_open()) {
        cerr << "Failed to open the file." << endl;
        return 1;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    string content = buffer.str();
    file.close();
    // cout << content << endl;


    TCPServer server("0.0.0.0", 7000);
    // todo 127.0.0.1 不能绑定
    server.openCallback = [](auto conn) {
        // cout << "this is open, " << format("ip: {}, port: {}", conn->peer_ip, conn->peer_port) << endl;
    };
    server.closeCallback = [](auto conn) {
        // cout << "this is close, " << format("ip: {}, port: {}", conn->peer_ip, conn->peer_port) << endl;
    };
    server.messageCallback = [&](auto conn, auto buf) {
        // cout << "<- " << buf->peek() << endl;
        buf->pop();

        conn->send(content);
        conn->shutdown();
    };

    server.run();
    return 0;
}