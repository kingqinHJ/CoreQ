#ifndef DIP_H
#define DIP_H


//https://medium.com/@oleksandra_shershen/solid-principles-implementation-and-examples-in-c-99f0d7e3e868
//定义：高层模块不应该依赖低层模块，两者都应该依赖抽象。抽象不应该依赖细节，细节应该依赖抽象。

//好处：降低模块间的耦合度，提高代码的灵活性和可测试性。



#include <string>
class Logger {
public:
    virtual void log(const std::string& message) = 0;
    virtual ~Logger() {}
};

class FileLogger : public Logger {
public:
    void log(const std::string& message) override {
        // 写入文件
    }
};

// 良好的设计：依赖抽象
class Application {
public:
    Application(Logger* l) : logger(l) {}
    void doSomething() {
        logger->log("Doing something");
    }
private:
    Logger* logger;
};

// 不良设计：违反DIP，直接依赖具体实现
class BadApplication {
public:
    void doSomething() {
        FileLogger logger;
        logger.log("Doing something");
    }
};



#endif // DIP_H
