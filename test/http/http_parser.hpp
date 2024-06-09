#pragma once
#include "easyloggingpp.h"
#include <memory>
#include <string>
#include <vector>
using namespace std;


struct State;

struct StateContext {
    bool is_request;
    int message_total;

    bool is_match_success;
    unique_ptr<State> state;
};


struct State {
    virtual void match(char c, StateContext&)=0;
};

struct Body: public State {
    int body_offset = 0;
    int body_len = 0;
    void match(char c, StateContext&) override {

    }
};

struct HeaderLine: public State {
    enum class part {
        key,
        split,
        value,
        end
    };

    part now_part = part::key;
    int now_idx = 0; 

    int key_offset = 0;
    int key_len = 0;
    int value_offset = 0;
    int value_len = 0;

    void match(char c, StateContext& context) override {
        switch (now_part) {
            case part::key:
                if (c == ':') {
                    now_part = part::split;
                    now_idx = 1;
                } else {
                    now_idx += 1;
                }
                break;
            case part::split:
                if (c != ' ') {
                    now_part = part::value;
                    now_idx = 1;
                } else {
                    now_idx += 1;
                }
                break;
            case part::value:
                if (c == '\r') {
                    now_part = part::end;
                    now_idx = 1;
                } else {
                    now_idx += 1;
                }
                break;
            case part::end:
                if (c == '\n') {
                    now_idx += 1;
                } else if (c == '\r' && now_idx == 2) {
                    now_idx += 1;
                } else if (c == '\n' && now_idx == 3) {
                    now_idx += 1;
                    context.state = unique_ptr<State>(new Body());
                } else {
                    context.is_match_success = false;
                }
                break;
            default:
                break; 
        }
    }
};

struct URL: public State {
    int protocol_offset = -1;
    int protocal_len = 0;
    int host_offset = -1;
    int host_len = 0;
    int port_offset = -1;
    int port_len = 0;
    int path_offset = -1;
    int path_len = 0;
    int query_offset = -1;
    int query_len = 0;

    enum class phrase {
        protocol,
        protocal_end,
        host,
        port,
        path,
        query
    };
    static string protocal_end_s;

    int now_idx = 0;
    phrase now_phrase = phrase::protocol;

    void match(char c, StateContext& context) override {
        // https://user:password@www.example.com:8080/path/to/resource?key1=value1&key2=value2#section1
        switch (now_phrase) {
            case phrase::protocol:
                if (c == ':') {
                    now_phrase = phrase::protocal_end;
                    now_idx = 1;
                } else {
                    now_idx += 1;
                }
                break;
            case phrase::protocal_end:
                if (now_idx == protocal_end_s.size()) {
                    now_phrase = phrase::host;
                    now_idx = 0;
                } else if (c == protocal_end_s[now_idx]) {
                    now_idx += 1;
                } else {
                    context.is_match_success = false;
                }
                break;
            case phrase::host:
                if (c == ':') {
                    now_phrase = phrase::port;
                    now_idx = 0;
                } else if (c == '/') {
                    now_phrase = phrase::query;
                    now_idx = 0;
                } else {
                    now_idx += 1;
                }
                break;
            case phrase::path:
                if (c == '?') {
                    now_phrase = phrase::query;
                    now_idx = 0;
                } else {
                    now_idx += 1;
                }
                break;
            case phrase::query:
                break;
            default:
                context.is_match_success = false;
        }
    }
};
string URL::protocal_end_s = "://";


struct Method: public State {
    enum class method_t {
        GET,
        POST
    };
    static vector<string> method_s;

    int now_idx;
    int now_method;

    void match(char c, StateContext& context) override {
        context.is_match_success = true;
        if (now_idx == 0) {
            if (c == 'G') {
                now_idx = 1;
                now_method = (int)method_t::GET;
            } else if (c == 'P') {
                now_idx = 1;
                now_method = (int)method_t::POST;
            } else {
                context.is_match_success = false;
            }
        } else if (now_idx < method_s[now_method].size()) {
            if (c == method_s[now_method][now_idx]) {
                now_idx += 1;
            } else {
                context.is_match_success = false;
            }
        } else {
            if (c == ' ') {
                context.state = unique_ptr<State>(new URL());
            } else {
                context.is_match_success = false;
            }
        }
    }
};
vector<string> Method::method_s = {"GET", "POST"};

struct Version: public State {
    enum class part {
        http,
        version0,
        version1
    };
    static vector<string> part_s;

    string http_version;
    part now_part = part::http;
    int now_idx = 0;
    void match(char c, StateContext& context) override {
        string& now_part_s = part_s[static_cast<int>(now_part)];

        switch (now_part) {
            case part::http:
                if (c == now_part_s.size()) {
                    if (c == '0') {
                        now_part = part::version0;
                    } else if (c == '1') {
                        now_part = part::version1;
                    } else {
                        context.is_match_success = false;
                    }
                    now_idx = 0;
                } else if (c == now_part_s[now_idx]) {
                    now_idx += 1;
                } else {
                    context.is_match_success = false;
                }
                break;
            case part::version0:
                if (c == now_part_s[now_idx]) {
                    now_idx += 1;
                    if (now_idx == now_part_s.size() - 1) {
                        http_version = "0.9";
                    }
                } else {
                    context.is_match_success = false;
                }
            case part::version1:
                if (c == now_part_s[now_idx]) {
                    now_idx += 1;
                    if (now_idx == 0) {
                        http_version = "1";
                    } else if (now_idx == now_part_s.size() - 1) {
                        http_version = "1.1";
                    }
                } else  {
                    context.is_match_success = false;
                }
                break;
            default:
                context.is_match_success = false;
        }
    }
};
vector<string> Version::part_s = {"http", "0.9", "1.1"};


struct Start: public State {
    void match(char c, StateContext& context) {
        if (context.is_request) {
            context.state = unique_ptr<State>(new Method());
        } else {
            context.state = unique_ptr<State>(new Version());
        }
    }
};

struct HTTPParser {
    
};