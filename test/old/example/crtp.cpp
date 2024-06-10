#include <iostream>

// 基类模板
template <typename Derived>
class Base {
public:
    void print() {
        // 通过静态类型转换获取派生类对象，并调用其特定方法
        static_cast<Derived*>(this)->specificPrint();
    }
};

// 派生类模板
class Derived1 : public Base<Derived1> {
public:
    void specificPrint() {
        std::cout << "Printing from Derived1" << std::endl;
    }
};

// 派生类模板
class Derived2 : public Base<Derived2> {
public:
    void specificPrint() {
        std::cout << "Printing from Derived2" << std::endl;
    }
};

int main() {
    // 奇异递归模板模式
    Base<Derived1> d1;
    Base<Derived2> d2;

    d1.print(); // 输出: Printing from Derived1
    d2.print(); // 输出: Printing from Derived2

    return 0;
}
