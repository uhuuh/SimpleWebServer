#include <functional>
#include <string>
#include <unordered_map>
#include "base.hpp"


class HTTPRequestMessage;
class HTTPResponseMessage;
class TCPServer2;
class HTTPServer2 {
public:
    using HTTPCallback = function<void(HTTPRequestMessage*, HTTPResponseMessage*)>;
    HTTPServer2(const string& ip, int port, int n_thread=1);
    void add_bind(const string& path, HTTPCallback cb);
    void run();
    void stop();
private:
    unique_ptr<TCPServer2> server;
    unordered_map<string, HTTPCallback> path_cb_map;
};