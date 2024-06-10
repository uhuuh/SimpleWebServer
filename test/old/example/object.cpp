#include <iostream>
using namespace std;

struct A {
    void f() {
        cout << "A" << endl;
    }
};

struct B: public A {
    void f() {
        cout << "B" << endl;
    }
};

int main() {
    A *x = new B();
    x->f(); // A
    B *x2 = new B();
    x2->f(); // B
    B x3;
    x3.f(); // B
    x2->A::f(); // A
    x3.A::f(); // A
    return 0;
}
