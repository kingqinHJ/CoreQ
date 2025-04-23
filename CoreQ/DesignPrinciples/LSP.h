#ifndef LSP_H
#define LSP_H
#include <stdexcept>

//定义：子类应该能够替换掉父类，并且不影响程序的正确性。即子类必须完全实现父类的方法，且不改变父类的行为。
//好处：确保子类可以无缝替换父类，提高代码的可靠性。

class Bird {
public:
    virtual void fly() = 0;
    virtual ~Bird() {}
};

class Sparrow : public Bird {
public:
    void fly() override {
        // 麻雀的飞行方式
    }
};

// 不良设计：违反LSP，Ostrich 不能替换 Bird
class Ostrich : public Bird {
public:
    void fly() override {
        throw std::logic_error("Ostrich can't fly");
    }
};

#endif // LSP_H
