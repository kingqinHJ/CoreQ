#include<iostream>
using namespace  std;


//https://medium.com/@oleksandra_shershen/solid-principles-implementation-and-examples-in-c-99f0d7e3e868
//解释：一个类只负责一个职责，变化原因单一。
//良好设计：将客户信息和账单计算分开，
class Customer {
public:
    void setDetails(const std::string& name, int id) {
        name_ = name;
        id_ = id;
    }
    std::string getName() const { return name_; }
    int getId() const { return id_; }
private:
    std::string name_;
    int id_;
};

class BillingCalculator {
public:
    double calculateBill(double amount) {
        return amount * 1.1; // 假设10%税率
    }
};


// //不良设计： 客户类同时处理信息和账单，增加维护难度
// class Customer {
// public:
//     void setDetails(const std::string& name, int id) {
//         name_ = name;
//         id_ = id;
//     }
//     std::string getName() const { return name_; }
//     int getId() const { return id_; }
//     double calculateBill(double amount) {
//         return amount * 1.1;
//     }
// private:
//     std::string name_;
//     int id_;
// };
