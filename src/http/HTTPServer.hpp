#include "TCPServer.hpp"
#include <string_view>
#include <unordered_map>
using namespace std;

class HTTPServer {
    unordered_map<string_view, int> ma;
    void main() {
        string_view a= "abc";
        if (a == "fdsa") cout << 1 << endl;
    }
};