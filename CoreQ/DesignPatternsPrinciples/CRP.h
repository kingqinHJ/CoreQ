#ifndef CRP_H
#define CRP_H

//定义：优先使用对象合成/聚合，而不是类继承来达到复用的目的。

//好处：提高代码的灵活性和可维护性，避免继承带来的僵化。
//https://en.wikipedia.org/wiki/Composition_over_inheritance
// 良好的设计：使用组合
class Engine {
public:
    void start() {
        // 启动引擎
    }
};

class Car {
public:
    Car(Engine* e) : engine(e) {}
    void drive() {
        engine->start();
        // 驾驶逻辑
    }
private:
    Engine* engine;
};

// 不良设计：违反CRP，过度使用继承
class BadCar : public Engine {
public:
    void drive() {
        start();
        // 驾驶逻辑
    }
};

#endif // CRP_H
