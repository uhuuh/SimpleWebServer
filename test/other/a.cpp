#include <functional>
#include <iostream>
using namespace std;

struct A {
    A() {
        cout << "A init" << endl;
    }
    ~A() {
        cout << "A destroy" << endl;
    } 
};
struct B {
    B() {
        cout << "B init" << endl;
    }
    ~B() {
        cout << "B destroy" << endl;
    }
};
struct Fa {
    A a;
    B b;
};

void test1() {
    Fa fa;
}

void test2() {
    function<void(void)> fun;
    fun();
}

int main() {
    test2();
    return 0;
}