#pragma once

class MyClass {
public:
    MyClass();
    ~MyClass();
    void doSomething();

private:
    class Impl;
    Impl* pImpl; // 指向实现类的指针
};

// MyClass.cpp
// #include "MyClass.h"

class MyClass::Impl {
public:
    void doSomethingPrivate() {
        // 实现细节
    }
};

MyClass::MyClass() : pImpl(new Impl()) {}

MyClass::~MyClass() {
    delete pImpl;
}

void MyClass::doSomething() { // 这里有重复代码，略显啰嗦
    pImpl->doSomethingPrivate();
}


