#ifndef CALLBACKDEMOWIDGET_H
#define CALLBACKDEMOWIDGET_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <functional>

class CallbackDemoWidget : public QWidget {
    Q_OBJECT

public:
    CallbackDemoWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("回调函数演示");
        resize(400, 600);

        QVBoxLayout *layout = new QVBoxLayout(this);

        // 标题标签
        QLabel *titleLabel = new QLabel("回调函数用法演示", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        layout->addWidget(titleLabel);

        // 结果显示
        resultLabel = new QLabel("Click the button to view the usage of different callback functions", this);
        resultLabel->setAlignment(Qt::AlignCenter);
        resultLabel->setFrameShape(QFrame::Panel);
        resultLabel->setFrameShadow(QFrame::Sunken);
        resultLabel->setMinimumHeight(100);
        layout->addWidget(resultLabel);

        // 各种回调函数演示按钮
        addDemoButton(layout, "signal and slots", [this]() { demoTraditionalSlot(); });
        addDemoButton(layout, "Lambda expression", [this]() { demoLambda(); });
        addDemoButton(layout, "std::function", [this]() { demoStdFunction(); });
        addDemoButton(layout, "函数指针", [this]() { demoFunctionPointer(); });
        addDemoButton(layout, "成员函数指针", [this]() { demoMemberFunctionPointer(); });
        addDemoButton(layout, "函数对象(Functor)", [this]() { demoFunctor(); });
        addDemoButton(layout, "std::bind", [this]() { demoStdBind(); });
        addDemoButton(layout, "QObject::connect", [this]() { demoQObjectConnect(); });

        layout->addStretch();
    }

private:
    QLabel *resultLabel;

    void addDemoButton(QVBoxLayout *layout, const QString &text, std::function<void()> callback) {
        QPushButton *button = new QPushButton(text, this);
        layout->addWidget(button);
        connect(button, &QPushButton::clicked, this, callback);
    }

    // 各种回调函数演示方法
    void demoTraditionalSlot() {
        resultLabel->setText("传统信号槽:\n"
                             "connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));\n"
                             "需要在类中定义 slots: void onButtonClicked();");
    }

    void demoLambda() {
        resultLabel->setText("Lambda 表达式:\n"
                             "connect(button, &QPushButton::clicked, [=]() {\n"
                             "    // 在此处理点击事件\n"
                             "});");

        // 实际演示一个Lambda
        int count = 0;
        auto lambda = [&count]() { return ++count; };
        qDebug() << "Lambda demo:" << lambda() << lambda();
    }

    void demoStdFunction() {
        resultLabel->setText("std::function:\n"
                             "std::function<void()> callback = []() { /* 处理逻辑 */ };\n"
                             "connect(button, &QPushButton::clicked, this, callback);");

        // 实际std::function演示
        std::function<int(int,int)> add = [](int a, int b) { return a + b; };
        qDebug() << "std::function demo:" << add(5, 3);
    }

    void demoFunctionPointer() {
        resultLabel->setText("函数指针:\n"
                             "void (*callback)() = &someFunction;\n"
                             "// 或直接作为参数传递\n"
                             "void processWithCallback(void (*callback)());");

        // 静态函数指针示例
        auto result = callWithFunctionPointer(&CallbackDemoWidget::staticCallback, 10);
        qDebug() << "Function pointer demo:" << result;
    }

    static int staticCallback(int value) {
        return value * 2;
    }

    int callWithFunctionPointer(int (*callback)(int), int value) {
        return callback(value);
    }

    void demoMemberFunctionPointer() {
        resultLabel->setText("成员函数指针:\n"
                             "void (ClassName::*memberFn)() = &ClassName::method;\n"
                             "(object.*memberFn)(); // 调用");

        // 成员函数指针示例
        using MemberFn = void (CallbackDemoWidget::*)();
        MemberFn fn = &CallbackDemoWidget::demoMemberFunctionPointer;
        (this->*fn)(); // 递归调用
    }

    void demoFunctor() {
        resultLabel->setText("函数对象(Functor):\n"
                             "class MyFunctor {\n"
                             "public:\n"
                             "    void operator()() { /* 处理逻辑 */ }\n"
                             "};\n"
                             "MyFunctor functor;\n"
                             "connect(button, &QPushButton::clicked, this, functor);");

        // 函数对象演示
        struct Multiplier {
            int factor;
            int operator()(int x) const { return x * factor; }
        };

        Multiplier times3{3};
        qDebug() << "Functor demo:" << times3(7);
    }

    void demoStdBind() {
        resultLabel->setText("std::bind:\n"
                             "auto callback = std::bind(&Class::method, this, std::placeholders::_1);\n"
                             "connect(button, &QPushButton::clicked, this, callback);");

        // std::bind演示
        auto boundFn = std::bind(&CallbackDemoWidget::multiply, this, 5, std::placeholders::_1);
        qDebug() << "std::bind demo:" << boundFn(7);
    }

    int multiply(int a, int b) {
        return a * b;
    }

    void demoQObjectConnect() {
        resultLabel->setText("Qt5+ QObject::connect:\n"
                             "connect(button, &QPushButton::clicked, this, &MyClass::onClicked);\n"
                             "connect(button, &QPushButton::clicked,\n"
                             "    [=](bool checked) { /* 处理点击 */ });");

        // 创建一个临时按钮来演示新的connect语法
        QPushButton *tempButton = new QPushButton(this);
        connect(tempButton, &QPushButton::clicked, this, [this]() {
            qDebug() << "Qt5+ connect syntax demonstration";
        });
        tempButton->deleteLater();
    }
};


#endif // CALLBACKDEMOWIDGET_H
