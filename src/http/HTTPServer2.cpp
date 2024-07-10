#include "HTTPServer2.hpp"
#include "TCPServer2.hpp"
#include "co_fun.hpp"
#include "HTTPMessageParser.hpp"
#include "Coroutine.hpp"
#include "EventLoopPool.hpp"


HTTPServer2::HTTPServer2(const string &ip, int port, int n_thread) {
    server = make_unique<TCPServer2>(ip, port, n_thread);
    server->user_cb = [this](int fd) {
        HTTPMessageParser parser(true);
        const int buf_size = 1024 * 4;
        int buf_ptr = 0;
        char buf[buf_size];
        while (true) {
            assertm(buf_ptr < buf_size);

            int r = co_read(fd, buf + buf_ptr, buf_size - buf_ptr);
            if (r < 0) {
                // log
            } else if (r == 0) {
                // log
            } else {
                parser.parser_message(buf, r);
                buf_ptr += r;

                if (parser.is_completed())
                    break;
            }
        }

        unique_ptr<HTTPRequestMessage> req = parser.get_request_message();
        HTTPResponseMessage res;
        auto it = path_cb_map.find(string(req->path));
        if (it != path_cb_map.end()) {
            it->second(req.get(), &res);
        } else {
            // todo
        }

        close(fd);
    };
}
void HTTPServer2::add_bind(const string &path, HTTPCallback cb) {
    path_cb_map[path] = cb;
}
void HTTPServer2::run() { server->run(); }
void HTTPServer2::stop() { server->stop(); }