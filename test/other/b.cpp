#include <chrono>
#include <ctime>
#include <iostream>
using namespace std;

int main() {
    // todo wsl上485左右, windows上600左右
    int count = 1e9;
    int a = 0;
    auto t1 = chrono::steady_clock::now();
    for (int i = 0; i < count; ++i) {
        a += 1;
    }
    auto t2 = chrono::steady_clock::now();
    auto tt =  chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
    cout << tt << endl;

    return 0;
}
