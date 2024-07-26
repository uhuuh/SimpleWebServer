#include <string>
using namespace std;

int createListenFd(const string& ip, int port);

void blockSIGPIPE();
