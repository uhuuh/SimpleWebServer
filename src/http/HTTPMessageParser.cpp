#include <cassert>
#include <cstdio>
#include <format>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include "HTTPMessageParser.hpp"
using namespace std;

string get_method_str(HTTPMethod me) {
    const char* mehtod_str[] = {
        "GET",
        "POST"
    };
    int si = sizeof(mehtod_str) / sizeof(decltype(mehtod_str[0]));
    auto me_i = static_cast<int>(me);
    assert(me_i <= si);
    return mehtod_str[me_i];
}

string get_version_str(HTTPVersion ve) {
    const char* version_str[] = {
        "HTTP/0.9",
        "HTTP/1.0",
        "HTTP/1.1",
    };
    int si = sizeof(version_str) / sizeof(decltype(version_str[0]));
    auto ve_i = static_cast<int>(ve);
    assert(ve_i <= si);
    return version_str[ve_i];
}

static unordered_map<int, string> http_code_map = {
  {100, "Continue"},
  {101, "Switching Protocols"},
  {102, "Processing"},
  {200, "OK"},
  {201, "Created"},
  {202, "Accepted"},
  {203, "Non-Authoritative Information"},
  {204, "No Content"},
  {205, "Reset Content"},
  {206, "Partial Content"},
  {207, "Multi-Status"},
  {208, "Already Reported"},
  {226, "IM Used"},
  {300, "Multiple Choices"},
  {301, "Moved Permanently"},
  {302, "Found"},
  {303, "See Other"},
  {304, "Not Modified"},
  {305, "Use Proxy"},
  {307, "Temporary Redirect"},
  {308, "Permanent Redirect"},
  {400, "Bad Request"},
  {401, "Unauthorized"},
  {402, "Payment Required"},
  {403, "Forbidden"},
  {404, "Not Found"},
  {405, "Method Not Allowed"},
  {406, "Not Acceptable"},
  {407, "Proxy Authentication Required"},
  {408, "Request Timeout"},
  {409, "Conflict"},
  {410, "Gone"},
  {411, "Length Required"},
  {412, "Precondition Failed"},
  {413, "Payload Too Large"},
  {414, "URI Too Long"},
  {415, "Unsupported Media Type"},
  {416, "Range Not Satisfiable"},
  {417, "Expectation Failed"},
  {421, "Misdirected Request"},
  {422, "Unprocessable Entity"},
  {423, "Locked"},
  {424, "Failed Dependency"},
  {426, "Upgrade Required"},
  {428, "Precondition Required"},
  {429, "Too Many Requests"},
  {431, "Request Header Fields Too Large"},
  {451, "Unavailable For Legal Reasons"},
  {500, "Internal Server Error"},
  {501, "Not Implemented"},
  {502, "Bad Gateway"},
  {503, "Service Unavailable"},
  {504, "Gateway Timeout"},
  {505, "HTTP Version Not Supported"},
  {506, "Variant Also Negotiates"},
  {507, "Insufficient Storage"},
  {508, "Loop Detected"},
  {510, "Not Extended"},
  {511, "Network Authentication Required"},
};

HTTPMessageWriter::HTTPMessageWriter(MessageWriteCallback cb): cb(cb) {}

void HTTPMessageWriter::write_message(HTTPRequestMessage& me) {
    string start_line = get_method_str(me.method) + " ";
    start_line += me.path;
    if (me.query.size() != 0) {
        start_line += "?";
        for (auto pa: me.query)  start_line += string(pa.first) + "=" + string(pa.second) + "&"; 
        start_line[start_line.size() - 1] = ' ';
    } else {
        start_line += " ";
    }
    start_line += get_version_str(me.version) + "\r\n";
    cb(start_line.c_str(), start_line.size());

    write_header_body(me.header, me.body);
}

void HTTPMessageWriter::write_message(HTTPResponseMessage& me) {
    string start_line = get_version_str(me.version) + " ";
    start_line += to_string(me.code) + " ";
    start_line += http_code_map[me.code] + "\r\n";
    cb(start_line.c_str(), start_line.size());

    write_header_body(me.header, me.body);
}

void HTTPMessageWriter::write_header_body(unordered_map<string, string_view> &header, string_view &body) {
    string header_str;
    for (auto& pa: header) header_str += string(pa.first) + ": " + string(pa.second) + "\r\n";
    cb(header_str.c_str(), header_str.size());

    if (body.size() != 0) {
        string empty_line = "\r\n";
        cb(empty_line.c_str(), empty_line.size());
        cb(body.begin(), body.size());
    }
}

struct StateBody: public State {
    int now_idx = 0;
    void match(HTTPMessageParser& context) override {
        auto l = context.now_len;
        if (now_idx == 0) context.body_pos.first = l - 1;
        else if (now_idx == context.total_len - 1) context.body_pos.second = l - context.body_pos.first;

        now_idx += 1;
    }
};

struct StateHeader: public State {
    enum class part_e {
        key,
        split,
        value,
        end
    };

    bool is_empty_line = false;
    int now_idx = 0;
    part_e now_part = part_e::key;
    int now_key_offset = 0;
    int now_key_len = 0;
    int now_value_offset = 0;
    int now_value_len = 0;

    void match(HTTPMessageParser& context) override {
        auto c = context.now_char;
        auto l = context.now_len;

        switch (now_part) {
            case part_e::key:
                if (c == '\r') now_part = part_e::end, is_empty_line = true;
                else if (now_idx == 0) now_key_offset = l - 1;
                else if (c == ':') now_part = part_e::split, now_key_len = l - now_key_offset - 1;

                break;
            case part_e::split:
                if (c != ' ') now_part = part_e::value, now_value_offset = l - 1;

                break;
            case part_e::value:
                if (c == '\r') {
                    now_part = part_e::end;
                    now_value_len = l - now_value_offset - 1;

                    string key(context.begin_ptr + now_key_offset, now_key_len);
                    context.header_pos[key] = {now_value_offset, now_value_len};
                    // todo 分块传输编码
                    if (key == "Content-Length") 
                        context.total_len = stoi(string(context.begin_ptr + now_value_offset, now_value_len));
                } 

                break;
            case part_e::end:
                if (c == '\n' && is_empty_line == true) context.state.reset(new StateBody());
                else if (c == '\n') now_part = part_e::key, now_idx = -1;
                else context.is_match = false;
                break;
            default:
                context.is_match = false;
        }

        now_idx += 1;
    }
};


struct StateDetail: public State {
    int now_idx = 0;
    bool is_prefix_r = false;
    void match(HTTPMessageParser& context) override {
        auto c = context.now_char;
        auto l = context.now_len;

        if (c == '\r') is_prefix_r = true;
        else if (is_prefix_r && c == '\n') context.detail_pos = {l - now_idx, now_idx + 1}, context.state.reset(new StateHeader());

        // 这里或许可以比较一下detail是否与http_code_map中的一致
        now_idx += 1;
    }
};

struct StateCode: public State {
    int now_idx = 0;
    int code_arr[3];
    int get_code() {
        int code = code_arr[0] * 100 + code_arr[1] * 10 + code_arr[2];
        if (http_code_map.find(code) != http_code_map.end()) {
            return code;
        } else {
            return -1;
        }
    }
    void match(HTTPMessageParser& context) override {
        auto c = context.now_char;
        if (c >= '0' && c <= '9' && now_idx < 3) code_arr[now_idx] = c - '0';
        else if (now_idx == 3 && c == ' ') context.code = get_code(), context.state.reset(new StateDetail());
        else context.is_match = false;

        if (context.code == -1) context.is_match = false;
        now_idx += 1;
    }
};

struct StateVersion: public State {
    int now_idx = 0;
    bool is_prefix_http0 = true;

    void match(HTTPMessageParser& context) override {
        auto c = context.now_char;
        auto l = context.now_len;

        switch (now_idx) {
            case 0:
                if (c != 'H') context.is_match = false;
                break;
            case 1:
                if (c != 'T') context.is_match = false;
                break;
            case 2:
                if (c != 'T') context.is_match = false;
                break;
            case 3:
                if (c != 'P') context.is_match = false;
                break;
            case 4:
                if (c != '/') context.is_match = false;
                break;
            case 5:
                if (c == '0') is_prefix_http0 = true;
                else if (c == '1') is_prefix_http0 = false;
                else context.is_match = false;
                break;
            case 6:
                if (c != '.') context.is_match = false;
                break;
            case 7:
                if (is_prefix_http0 && c == '9') context.version = HTTPVersion::V0_9; 
                else if (!is_prefix_http0 && c == '0') context.version = HTTPVersion::V1_0;
                else if (!is_prefix_http0 && c == '1') context.version = HTTPVersion::V1_1;
                else context.is_match = false;
                break;
            case 8:
                if (context.is_request && c == '\r') ;
                else if (!context.is_request && c == ' ') context.state.reset(new StateCode());
                else context.is_match = false;
                break;
            case 9:
                if (context.is_request && c == '\n') context.state.reset(new StateHeader());
                else context.is_match = false;
                break;
            default:
                context.is_match = false;
        }

        now_idx += 1;
    }
};

struct StateURL: public State {
    int now_idx = 0;
    int now_key_offset = -1;
    int now_key_len = -1;
    int now_value_offset = -1;
    int now_value_len = -1;
    bool has_query = false;

    void match(HTTPMessageParser& context) override {
        auto c = context.now_char;
        auto l = context.now_len;

        if (c == '?') {
            context.path_pos.second = l - context.path_pos.first - 1;
            now_key_offset = l;
            has_query = true;
        } else if (c == '=') {
            now_key_len = l - now_key_offset - 1;
            now_value_offset = l;
        } else if (c == '&') {
            now_value_len = l - 1 - now_value_offset;
            context.query_pos[string(context.begin_ptr + now_key_offset, now_key_len)] = {now_value_offset, now_value_len};

            now_key_offset = l;
            now_key_len = -1;
            now_value_offset = -1;
            now_value_len = -1;
        } else if (c == ' ') {
            if (!has_query) {
                context.path_pos.second = l - context.path_pos.first - 1;
            } else {
                now_value_len = l - now_value_offset - 1;
                assert(now_key_offset != -1 && now_key_len != -1 && now_value_offset != -1 && now_value_len != -1);
                context.query_pos[string(context.begin_ptr + now_key_offset, now_key_len)] = {now_value_offset, now_value_len};
            }

            context.state = unique_ptr<State>(new StateVersion());
        } else {
            if (now_idx == 0) {
                context.path_pos.first = l;
            }
        }

        now_idx += 1;
    }
};


struct StateMethod: public State {
    int now_idx;

    void match(HTTPMessageParser& context) override {
        context.is_match = true;
        auto c = context.now_char;
        auto method_s = get_method_str(context.method);

        if (now_idx == 0) {
            if (c == 'G') {
                context.method = HTTPMethod::GET;
            } else if (c == 'P') {
                context.method = HTTPMethod::POST;
            } else {
                context.is_match = false;
            }
        } else if (now_idx < method_s.size()) {
            if (c != method_s[now_idx]) {
                context.is_match = false;
            }
        } else {
            if (c == ' ') {
                context.state.reset(new StateURL());
            } else {
                context.is_match = false;
            }
        }

        now_idx += 1;
    }
};



HTTPMessageParser::HTTPMessageParser(bool is_request): is_request(is_request) {
    if (is_request) {
        state.reset(new StateMethod());
    } else {
        state.reset(new StateVersion());
    }
}

bool  HTTPMessageParser::is_completed() {
    auto p = dynamic_cast<StateBody*>(state.get());
    if (p != nullptr && p->now_idx == total_len) return true; // 因为match中now_idx最后加一，故这里是now_idx = total_len而不是now_idx = total_len - 1

    auto p2 = dynamic_cast<StateHeader*>(state.get());
    if (p2 != nullptr && p2->now_part == StateHeader::part_e::key && p2->now_idx == 0) return true;

    return false;
}

bool HTTPMessageParser::parser_message(const char* str, int len) {
    if (!is_match) return false;
    if (is_completed()) return false;

    begin_ptr = str - now_len;

    for (int i = 0; i < len; ++i) {
        now_char = *(str + i);
        now_len += 1;

        state->match(*this);

        assert(is_match);
        if (!is_match) return false;
    }
    return true;
}

unique_ptr<HTTPRequestMessage>  HTTPMessageParser::get_request_message() {
    assert(is_request && is_completed());

    unique_ptr<HTTPRequestMessage> me = make_unique<HTTPRequestMessage>();
    me->method = method;
    me->path = string_view(begin_ptr + path_pos.first, path_pos.second);
    me->version = version;
    for (auto pa: query_pos)  me->query[pa.first] = string_view(begin_ptr + pa.second.first, pa.second.second); 
    for (auto pa: header_pos)  me->header[pa.first] = string_view(begin_ptr + pa.second.first, pa.second.second); 
    me->body = string_view(begin_ptr + body_pos.first, body_pos.second);
    return me;
}

unique_ptr<HTTPResponseMessage> HTTPMessageParser::get_response_message() {
    assert(!is_request && is_completed());

    unique_ptr<HTTPResponseMessage> me = make_unique<HTTPResponseMessage>();
    me->version = version;
    me->code = code;
    me->detail = string_view(begin_ptr + detail_pos.first, detail_pos.second);
    for (auto pa: header_pos) me->header[pa.first] = string_view(begin_ptr + pa.second.first, pa.second.second);
    me->body = string_view(begin_ptr + body_pos.first, body_pos.second);
    return me;
}

