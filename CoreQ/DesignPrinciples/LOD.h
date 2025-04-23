#ifndef LOD_H
#define LOD_H

//定义：一个对象应该对其他对象有尽可能少的了解。只与直接的朋友通信，不与“陌生人”交流。

//好处：降低对象间的耦合，提高代码的可维护性。

// 良好的设计：只与直接朋友通信
class Wallet {
public:
    double getBalance() const { return balance; }
private:
    double balance = 100.0;
};

class Person {
public:
    void pay(double amount) {
        if (wallet.getBalance() >= amount) {
            // 支付逻辑
        }
    }
private:
    Wallet wallet;
};

// 不良设计：违反LoD，直接访问内部结构
class BadPerson {
public:
    void pay(double amount, Wallet& w) {
        if (w.balance >= amount) { // 直接访问 Wallet 的私有成员
            // 支付逻辑
        }
    }
};


#endif // LOD_H
