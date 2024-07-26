#pragma once
#include <cassert>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
using namespace std;

enum class HTTPMethod {
    GET,
    POST,
};



enum class HTTPVersion {
    V0_9,
    V1_0,
    V1_1,
    V2,
    V3
};

struct HTTPRequestMessage {
    HTTPMethod method;
    string_view path;
    unordered_map<string, string_view> query;
    HTTPVersion version;
    unordered_map<string, string_view> header;
    string_view body;
};

struct HTTPResponseMessage {
    HTTPVersion version;
    int code;
    string_view detail;
    unordered_map<string, string_view> header;
    string_view body;
};


using PII = pair<int, int>;
struct HTTPMessageParser;
struct State {
    virtual void match(HTTPMessageParser&)=0;
    virtual ~State() = default;
};

struct HTTPMessageParser {
    const bool is_request;
    // bool is_completed = false;
    bool is_match = true;
    int fail_code = -1;

    int total_len = 0; // 头部中没有内容长度字段，内容长度默认为0
    int now_len = 0;
    char now_char = 0;
    const char* begin_ptr = nullptr;
    unique_ptr<State> state;
    State* next_state = nullptr;

    HTTPMethod method = HTTPMethod::GET;
    PII path_pos; 
    unordered_map<string, PII> query_pos;
    HTTPVersion version;
    int code;
    PII detail_pos;
    unordered_map<string, PII> header_pos;
    PII body_pos;

    HTTPMessageParser(bool is_request);
    bool is_completed();
    bool parser_message(const char* str, int len);
    unique_ptr<HTTPRequestMessage> get_request_message();
    unique_ptr<HTTPResponseMessage> get_response_message();
};

using MessageWriteCallback = function<void(const char*, int)>;

class HTTPMessageWriter {
public:
    HTTPMessageWriter(MessageWriteCallback cb);
    void write_error_response_message(int code);
    void write_message(HTTPRequestMessage& me);
    void write_message(HTTPResponseMessage& me);
private:
    void write_header_body(unordered_map<string, string_view> &header, string_view &body);
    MessageWriteCallback cb;
};


