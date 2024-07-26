#include "HTTPServer2.hpp"
#include "Logger.hpp"
#include "TCPServer2.hpp"
#include "co_fun.hpp"
#include "HTTPMessageParser.hpp"
#include "Coroutine.hpp"
#include "Buffer.hpp"
#include <cstdio>
#include <string_view>
#include <utility>
#include "tcp_user.hpp"


HTTPServer2::HTTPServer2(const string &ip, int port, int n_thread) {
    server = make_unique<TCPServer2>(ip, port, n_thread);
    server->user_cb = [this] (int fd) {
        LOG_DEBUG("conn fd=%d", fd);

        Buffer buf;
        int code = 200;
        HTTPMessageParser parser(true);
        HTTPMessageWriter writer([&] (const char* str, int size) {
            buf.push(string_view(str, size));
            int n = buf.pop_to(fd, co_write);
            if (n < 0) code = -1;
        });
        unique_ptr<HTTPRequestMessage> req;
        HTTPResponseMessage res;
        decltype(path_cb_map)::iterator it;

        while (true) {
            int r = buf.push_from(fd, co_read);
            if (r <= 0) {
                code = -1;
                break;
            } else {
                if (!parser.parser_message(buf.peek().begin(), r)) {
                    code = 400;
                    break;
                } else if (parser.is_completed()) {
                    req = parser.get_request_message();
                    it = path_cb_map.find(string(req->path));
                    if (it == path_cb_map.end()) {
                        code = 404;
                    } 
                    break;
                }
            }
        }


        if (code == 200) {
            it->second(req.get(), &res);
            writer.write_message(res);
        } else if (code >= 0) {
            writer.write_error_response_message(code);

            // HTTPMessageWriter debug_writer([] (const char* str, int size) {
            //     for (int i = 0; i < size; ++i) putchar(str[i]);
            // });
        }

        if (code == -1) LOG_ERROR("http client close conn");
        co_close(fd);
    };
}
void HTTPServer2::add_bind(const string &path, HTTPCallback cb) {
    path_cb_map[path] = cb;
}
void HTTPServer2::run() { 
    blockSIGPIPE(); // write在fd关闭时抛出信号, 禁止这个信号防止程序终止

    server->run(); 
}
void HTTPServer2::stop() { server->stop(); }