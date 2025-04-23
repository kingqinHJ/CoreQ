#ifndef ISP_H
#define ISP_H

#include<iostream>
//https://medium.com/@oleksandra_shershen/solid-principles-implementation-and-examples-in-c-99f0d7e3e868
//定义：客户端不应该依赖它不需要的接口。即一个类对另一个类的依赖应该建立在最小的接口上。

//好处：减少接口的臃肿，提高灵活性，避免“胖接口”。
using namespace  std;

// 良好的设计：拆分接口
class Printable {
public:
    virtual void print() = 0;
    virtual ~Printable() {}
};

class Scannable {
public:
    virtual void scan() = 0;
    virtual ~Scannable() {}
};

class Printer : public Printable {
public:
    void print() override {
        // 打印逻辑
    }
};

// 不良设计：违反ISP，一个大接口
class Machine {
public:
    virtual void print() = 0;
    virtual void scan() = 0;
    virtual ~Machine() {}
};

class BadPrinter : public Machine {
public:
    void print() override {
        // 打印逻辑
    }
    void scan() override {
        throw std::logic_error("Not implemented");
    }
};



#endif // ISP_H
