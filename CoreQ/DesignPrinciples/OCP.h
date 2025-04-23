#ifndef OCP_H
#define OCP_H

//解释： 对扩展开放，对修改关闭，通过继承扩展功能。
//良好设计： 使用抽象类扩展形状类型，无需修改现有代码。
class Shape {
public:
    virtual double area() const = 0;
};
class Rectangle : public Shape {
public:
    Rectangle(double width, double height) : width_(width), height_(height) {}
    double area() const override { return width_ * height_; }
private:
    double width_, height_;
};

// //不良设计： 修改现有类添加新形状，增加出错风险。
// class Shape {
// public:
//     double area() const {
//         if (type_ == "rectangle") return width_ * height_;
//         else if (type_ == "circle") return 3.14 * radius_ * radius_;
//         return 0;
//     }
//     void setRectangle(double width, double height) { type_ = "rectangle"; width_ = width; height_ = height; }
// private:
//     std::string type_;
//     double width_, height_, radius_;
// };

#endif // OCP_H
