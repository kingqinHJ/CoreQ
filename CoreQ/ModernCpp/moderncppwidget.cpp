#include "moderncppwidget.h"
#include <QDebug>
#include <QStringList>
#include <QFont>
#include <QTime>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <string>

namespace Modern {
    void foo(int n) {}
    void foo(char* p) {}
}

// CppFeatureTable 类实现
CppFeatureTable::CppFeatureTable(QWidget* parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    tableWidget = new QTableWidget(this);
    
    // 设置表格列数和列标题
    tableWidget->setColumnCount(4);
    QStringList headers;
    headers << "Featrue Name" << "Classify" << "Usage Scenario" << "execute";
    tableWidget->setHorizontalHeaderLabels(headers);
    
    // 设置表格样式
    tableWidget->setAlternatingRowColors(true);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
    tableWidget->setSelectionBehavior(QTableWidget::SelectRows);
    
    // 连接单元格点击信号
    connect(tableWidget, &QTableWidget::cellClicked, this, &CppFeatureTable::onCellClicked);
    
    layout->addWidget(tableWidget);
    setLayout(layout);
}

void CppFeatureTable::addFeature(const QString& name, const QString& category, const QString& useCase, 
                               std::function<void(QTextEdit*)> demoFunction)
{
    // 创建特性信息
    FeatureInfo feature;
    feature.name = name;
    feature.category = category;
    feature.useCase = useCase;
    feature.demoFunction = demoFunction;
    features.append(feature);
    
    // 更新表格行数
    int row = tableWidget->rowCount();
    tableWidget->setRowCount(row + 1);
    
    // 填充表格内容
    tableWidget->setItem(row, 0, new QTableWidgetItem(name));
    tableWidget->setItem(row, 1, new QTableWidgetItem(category));
    tableWidget->setItem(row, 2, new QTableWidgetItem(useCase));
    
    QPushButton* runButton = new QPushButton(u8"运行", this);
    runButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border-radius: 4px; padding: 5px; }");
    tableWidget->setCellWidget(row, 3, runButton);
    
    connect(runButton, &QPushButton::clicked, [this, row]() {
        const FeatureInfo& info = features[row];
        emit featureClicked(info.name, info.category, info.demoFunction);
    });
}

void CppFeatureTable::onCellClicked(int row, int column)
{
    if (column == 3 && row < features.size()) {
        const FeatureInfo& info = features[row];
        emit featureClicked(info.name, info.category, info.demoFunction);
    }
}

// ModernCppWidget 类实现
ModernCppWidget::ModernCppWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
    createFeatureTables();
}

ModernCppWidget::~ModernCppWidget()
{
}

void ModernCppWidget::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    
    // 标题
    QLabel* titleLabel = new QLabel("Modern C++", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // 输出文本框
    outputTextEdit = new QTextEdit(this);
    outputTextEdit->setReadOnly(true);
    outputTextEdit->setMinimumHeight(200);
    outputTextEdit->setStyleSheet("QTextEdit { font-family: Consolas, Courier, monospace; background-color: #f8f8f8; border: 1px solid #ddd; padding: 8px; }");
    
    // 按钮布局
    buttonLayout = new QHBoxLayout();
    QString buttonTitles[] = {"C++11", "C++14", "C++17", "C++20", "C++23"};
    QString buttonColors[] = {"#3498db", "#9b59b6", "#e74c3c", "#2ecc71", "#f39c12"};
    
    for (int i = 0; i < 5; ++i) {
        buttons[i] = new QPushButton(buttonTitles[i], this);
        buttons[i]->setStyleSheet(QString("QPushButton { background-color: %1; color: white; "
                                           "border-radius: 4px; padding: 8px; font-weight: bold; }")
                                    .arg(buttonColors[i]));
        buttonLayout->addWidget(buttons[i]);
        
        connect(buttons[i], &QPushButton::clicked, [this, i]() {
            onButtonClicked(i);
        });
    }
    
    // 堆叠窗口部件
    stackedWidget = new QStackedWidget(this);
    
    // 添加到主布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(outputTextEdit);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(stackedWidget);
    
    setLayout(mainLayout);
    setMinimumSize(800, 600);
}

void ModernCppWidget::createFeatureTables()
{
    // 创建五个表格页面
    cpp11Table = new CppFeatureTable(this);
    cpp14Table = new CppFeatureTable(this);
    cpp17Table = new CppFeatureTable(this);
    cpp20Table = new CppFeatureTable(this);
    cpp23Table = new CppFeatureTable(this);
    
    // 添加到堆叠窗口部件
    stackedWidget->addWidget(cpp11Table);
    stackedWidget->addWidget(cpp14Table);
    stackedWidget->addWidget(cpp17Table);
    stackedWidget->addWidget(cpp20Table);
    stackedWidget->addWidget(cpp23Table);
    
    // 连接各表格的点击信号
    connect(cpp11Table, &CppFeatureTable::featureClicked, this, &ModernCppWidget::onFeatureClicked);
    connect(cpp14Table, &CppFeatureTable::featureClicked, this, &ModernCppWidget::onFeatureClicked);
    connect(cpp17Table, &CppFeatureTable::featureClicked, this, &ModernCppWidget::onFeatureClicked);
    connect(cpp20Table, &CppFeatureTable::featureClicked, this, &ModernCppWidget::onFeatureClicked);
    connect(cpp23Table, &CppFeatureTable::featureClicked, this, &ModernCppWidget::onFeatureClicked);
    
    // 设置各版本特性
    setupCpp11Features(cpp11Table);
    setupCpp14Features(cpp14Table);
    setupCpp17Features(cpp17Table);
    setupCpp20Features(cpp20Table);
    setupCpp23Features(cpp23Table);
    
    // 默认显示C++11页面
    stackedWidget->setCurrentIndex(0);
    buttons[0]->setChecked(true);
}

void ModernCppWidget::onButtonClicked(int index)
{
    stackedWidget->setCurrentIndex(index);
}

void ModernCppWidget::onFeatureClicked(const QString& featureName, const QString& category, std::function<void(QTextEdit*)> demoFunction)
{
    outputTextEdit->clear();
    outputTextEdit->append(QString(QString::fromUtf8(u8"<h3>特性: %1 (%2)</h3>")).arg(featureName).arg(category));
    outputTextEdit->append(u8"<p>执行结果:</p>");
    
    try {
        demoFunction(outputTextEdit);
    } catch (const std::exception& e) {
        outputTextEdit->append(QString(u8"<span style='color: red;'>发生异常: %1</span>").arg(e.what()));
    } catch (...) {
        outputTextEdit->append(u8"<span style='color: red;'>发生未知异常</span>");
    }
}

void ModernCppWidget::setupCpp11Features(CppFeatureTable* table)
{
    // Lambda表达式
    table->addFeature(
        "Lambda",
        "核心语言增强",
        u8"创建简单的匿名函数,用于就地定义简单的函数",
        [](QTextEdit* output) {
            auto sum = [](int a, int b) { return a + b; };
            output->append(QString(u8"Lambda计算 5 + 3 = %1").arg(sum(5, 3)));
            
            std::vector<int> numbers = {1, 2, 3, 4, 5};
            int total = 0;
            std::for_each(numbers.begin(), numbers.end(), [&total](int n) { total += n; });
            output->append(QString(u8"数组元素总和: %1").arg(total));
        }
    );
    
    // 移动语义
    table->addFeature(
        "移动语义",
        "核心语言增强",
        "避免不必要的对象拷贝，提高性能",
        [](QTextEdit* output) {
            auto now = []() -> QString {
                return QTime::currentTime().toString("hh:mm:ss.zzz");
            };

            output->append(QString("开始时间: %1").arg(now()));

            std::vector<std::string> v1(1000000, "测试");
            output->append(QString("创建大向量 v1，大小: %1").arg(v1.size()));

            output->append(QString("复制前时间: %1").arg(now()));
            std::vector<std::string> v2 = v1; // 拷贝操作
            output->append(QString("拷贝完成，v2大小: %1").arg(v2.size()));

            output->append(QString("移动前时间: %1").arg(now()));
            std::vector<std::string> v3 = std::move(v1); // 移动操作
            output->append(QString("移动完成，v3大小: %1, v1大小: %2").arg(v3.size()).arg(v1.size()));
            output->append(QString("结束时间: %1").arg(now()));
        }
        );

    // const和constexpr的区别
    //(1)const
    //const 关键字主要用于声明一个只读变量，它强调的是运行时的常量性，即一旦变量被初始化后，在程序运行期间其值不能被修改。
    //可以用于修饰普通变量、函数参数、函数返回值等，用于保证变量在运行时不会被意外修改。
    //(2)constexpr
    //constexpr 关键字用于声明常量表达式，它强调的是编译时的常量性，编译器会在编译阶段对 constexpr 修饰的表达式进行求值，生成一个编译时常量。
    //主要用于需要编译时常量的场景，如数组大小、枚举值、模板参数等。
    //常量表达式
    table->addFeature(
        "constexpr - 常量表达式",
        "核心语言增强",
        "允许将计算过程放在编译期进行，可用于定义编译期常量和调用编译期函数。",
        [](QTextEdit* output) {
            constexpr int f5 = factorial(5); // 在编译期计算出 120
            int arr[f5]; // 合法，因为 f5 是编译期常量
            output->append(QString("arr的长度为:%1").arg(sizeof(arr)));
        }
    );

    //用途	描述
    //通用函数	类型安全的打印、日志、参数遍历等
    //元编程	类型列表、递归模板、trait 分析
    //转发器	完美转发，构建包装器、函数调用器
    //多继承	构建 tuple、variant、访问器等
    //构造器	通用类构造、初始化方法
    //消息系统	实现观察者、事件分发系统
    table->addFeature(
        "可变参数模板",
        "核心语言增强",
        "允许模板接受任意数量的模板参数，常用于实现 printf 或 tuple 这类功能。",
        [](QTextEdit* output){
            print("hello",1,1.123,'c');  
        }
    );

    // 智能指针
    table->addFeature(
        u8"智能指针",
        "STL",
        "Automatic memory management to prevent memory leaks",
        [](QTextEdit* output) {
            // std::unique_ptr示例
            output->append(u8"=== std::unique_ptr (独占所有权) ===");
            std::unique_ptr<int> ptr1 = std::make_unique<int>(42);
            output->append(QString(u8"unique_ptr值: %1").arg(*ptr1));
            output->append(u8"特点: 独占所有权，不可复制，只能移动");
            
            // 演示unique_ptr的移动语义
            std::unique_ptr<int> ptr1_moved = std::move(ptr1);
            output->append(u8"移动后:");
            output->append(QString(u8"原ptr1是否为空: %1").arg(ptr1 ? "否" : "是"));
            output->append(QString(u8"新ptr1_moved值: %1").arg(*ptr1_moved));
            
            // std::shared_ptr示例
            output->append(u8"\n=== std::shared_ptr (共享所有权) ===");
            std::shared_ptr<int> ptr2 = std::make_shared<int>(100);
            output->append(QString(u8"shared_ptr值: %1").arg(*ptr2));
            output->append(QString(u8"初始引用计数: %1").arg(ptr2.use_count()));
            
            // 创建多个shared_ptr指向同一对象
            std::shared_ptr<int> ptr3 = ptr2;
            output->append(u8"创建第二个shared_ptr指向同一对象:");
            output->append(QString(u8"引用计数: %1").arg(ptr2.use_count()));
            output->append(QString(u8"两个指针指向同一对象: %1").arg(ptr2.get() == ptr3.get() ? "是" : "否"));
 
            // std::weak_ptr示例
            output->append(u8"\n=== std::weak_ptr (弱引用) ===");
            std::weak_ptr<int> weakPtr = ptr2;
            output->append(QString(u8"weak_ptr不影响引用计数: %1").arg(ptr2.use_count()));
            output->append(QString(u8"weak_ptr是否过期: %1").arg(weakPtr.expired() ? "是" : "否"));
            
            // 通过weak_ptr安全访问对象
            if (auto locked = weakPtr.lock()) {
                output->append(QString(u8"通过weak_ptr访问值: %1").arg(*locked));
                output->append(QString(u8"临时shared_ptr引用计数: %1").arg(locked.use_count()));
            }
            
            output->append(u8"\n使用场景总结:");
            output->append(u8"• unique_ptr: 工厂模式、RAII资源管理、类成员管理");
            output->append(u8"• shared_ptr: 多对象共享资源、异步操作、观察者模式");
            output->append(u8"• weak_ptr: 解决循环引用、观察对象生命周期、缓存系统");
        }
    );
    
    // nullptr关键字
    table->addFeature(
        "nullptr",
        "新增关键字与标识符",
        "新的空指针常量，解决了之前用 NULL 或 0 带来的类型不安全和重载问题。",
        [](QTextEdit* output) {
            //foo(NULL); // 编译错误：二义性
            Modern::foo(nullptr); // 正确调用 foo(char*)
            output->append(QString("新的空指针常量，解决了之前用 NULL 或 0 带来的类型不安全和重载问题。"));
        }
    );

    // auto、decltype关键字
    table->addFeature(
        "auto,decltype",
        "新增关键字与标识符",
        "Automatic type derivation and simplified variable declaration",
        [](QTextEdit* output) {
            auto i = 42;  // int
            auto d = 3.14; // double
            auto s = "hello"; // const char*
            
            output->append(QString("auto整数: %1 (类型: int)").arg(i));
            output->append(QString("auto浮点数: %1 (类型: double)").arg(d));
            output->append(QString("auto字符串: %1 (类型: const char*)").arg(s));
            
            std::vector<int> vec = {1, 2, 3, 4, 5};
            output->append("\n使用auto遍历容器:");
            for (const auto& v : vec) {
                output->append(QString("  元素: %1").arg(v));
            }

            // decltype关键字
            int x = 10;
            decltype(x) y = 20;
            output->append(QString("decltype(x) y = %1").arg(y));

            auto func = [](int a, int b) -> int { return a + b; };
            decltype(func) func2 = func;
            output->append(QString("decltype(func) func2 = %1").arg(func2(1, 2)));
        }
    );
    
    // 基于范围的for循环
    table->addFeature(
        "基于范围的for循环", 
        "语法糖与便利性改进",
        "Simplify container traversal",
        [](QTextEdit* output) {
            std::vector<std::string> fruits = {"苹果", "香蕉", "橙子", "葡萄"};
            
            output->append("使用传统for循环:");
            for (size_t i = 0; i < fruits.size(); ++i) {
                output->append(QString("  水果[%1]: %2").arg(i).arg(QString::fromStdString(fruits[i])));
            }
            
            output->append("\n使用基于范围的for循环:");
            for (const auto& fruit : fruits) {
                output->append(QString("  水果: %1").arg(QString::fromStdString(fruit)));
            }
        }
    );
}

void ModernCppWidget::setupCpp14Features(CppFeatureTable* table)
{
    // 泛型lambda
    table->addFeature(
        "Generic Lambda Expression",
        "C++ Feature",
        " Creating a Lambda Expression that can Handle Multiple Types.",
        [](QTextEdit* output) {
            // C++14中的泛型lambda
            auto genericSum = [](auto a, auto b) { return a + b; };
            
            output->append(QString("泛型Lambda计算整数: 5 + 3 = %1").arg(genericSum(5, 3)));
            output->append(QString("泛型Lambda计算浮点数: 3.14 + 2.71 = %1").arg(genericSum(3.14, 2.71)));
            output->append(QString("泛型Lambda计算字符串: hello + world = %1").arg(
                              QString::fromStdString(genericSum(std::string("hello"), std::string("world")))));
        }
    );
    
    // // 变量模板
    // table->addFeature(
    //     "变量模板",
    //     "C++ feature",
    //     "Create a generic variable that can adjust its value based on the type",
    //     [](QTextEdit* output) {
    //         // 演示C++14变量模板
    //         output->append("变量模板演示:");
            
    //         // 定义一个变量模板，用于演示目的
    //         template<typename T>
    //         constexpr T pi = T(3.1415926535897932385);
            
    //         output->append(QString("pi<int> = %1").arg(pi<int>));
    //         output->append(QString("pi<float> = %1").arg(pi<float>));
    //         output->append(QString("pi<double> = %1").arg(pi<double>));
    //     }
    // );
    
    // 改进的constexpr
    table->addFeature(
        "改进的constexpr",
        "c++ feature",
        "扩展了编译期计算能力", 
        [](QTextEdit* output) {
            // C++14中可以在constexpr函数中使用更多语言特性
            constexpr auto factorial = [](int n) {
                int result = 1;
                for (int i = 1; i <= n; ++i) {
                    result *= i;
                }
                return result;
            };
            
            output->append("Calculate factorial");
            output->append(QString("5! = %1").arg(factorial(5)));
            output->append(QString("10! = %1").arg(factorial(10)));
        }
    );
}

void ModernCppWidget::setupCpp17Features(CppFeatureTable* table)
{
    // 结构化绑定
    table->addFeature(
        "Structured binding",
        "C++ featrue",
        "简化多返回值的获取", 
        [](QTextEdit* output) {
            // 使用结构化绑定
            std::pair<int, std::string> person = {28, "张三"};
            auto [age, name] = person;
            
            output->append("使用结构化绑定获取pair的元素：");
            output->append(QString("姓名: %1, 年龄: %2").arg(QString::fromStdString(name)).arg(age));
            
            std::map<std::string, int> scores = {{"语文", 90}, {"数学", 85}, {"英语", 95}};
            output->append("\nTraverse using structured binding map");
            for (const auto& [subject, score] : scores) {
                output->append(QString("科目: %1, 成绩: %2").arg(QString::fromStdString(subject)).arg(score));
            }
        }
    );
    
    // if/switch初始化语句
    table->addFeature(
        "if/switch Initialization statement",
        "C++ featrue",
        "在条件语句中定义临时变量", 
        [](QTextEdit* output) {
            output->append("if语句初始化：");
            // C++17的if初始化语句
            if (int value = 42; value > 10) {
                output->append(QString("值 %1 大于10").arg(value));
            } else {
                output->append(QString("值 %1 不大于10").arg(value));
            }
            
            output->append("\nswitch语句初始化：");
            switch (int dice = rand() % 6 + 1; dice) {
                case 1:
                    output->append(QString("result: %1 - unluckily").arg(dice));
                    break;
                case 6:
                    output->append(QString("result: %1 - luckily").arg(dice));
                    break;
                default:
                    output->append(QString("result: %1 - just so so").arg(dice));
            }
        }
    );
    
    // // std::optional
    // table->addFeature(
    //     "std::optional",
    //     "std",
    //     "Represents values that may exist or may not exist",
    //     [](QTextEdit* output) {
    //         // 模拟std::optional功能
    //         output->append("std::optional示例");
            
    //         auto findValue = [](int searchKey) -> std::optional<std::string> {
    //             std::map<int, std::string> data = {
    //                 {1, QString::fromLocal8Bit("一")}, {2, QString::fromLocal8Bit("二")}, {3, QString::fromLocal8Bit("三")}, {5, QString::fromLocal8Bit("五")}
    //             };
                
    //             auto it = data.find(searchKey);
    //             if (it != data.end()) {
    //                 return it->second;
    //             }
    //             return std::nullopt;
    //         };
            
    //         auto printResult = [output](int key, const std::optional<std::string>& result) {
    //             if (result) {
    //                 output->append(QString("键 %1 对应的值: %2").arg(key).arg(QString::fromStdString(*result)));
    //             } else {
    //                 output->append(QString("键 %1 没有对应的值").arg(key));
    //             }
    //         };
            
    //         printResult(1, findValue(1));
    //         printResult(4, findValue(4));
    //         printResult(5, findValue(5));
    //     }
    // );
}

void ModernCppWidget::setupCpp20Features(CppFeatureTable* table)
{
    // 概念和约束(Concepts)
    table->addFeature(
        "概念和约束(Concepts)", 
        "C++ feature",
        "Enhance the constraint ability of template parameters",
        [](QTextEdit* output) {
            output->append("C++20概念和约束 - 代码示例:");
            output->append(R"(
// 定义一个概念
template<typename T>
concept Numerical = std::is_arithmetic_v<T>;

// 使用概念约束函数模板
template<Numerical T>
T add(T a, T b) {
    return a + b;
}

// 使用
int result1 = add(5, 3);       // 正确
double result2 = add(3.14, 2.71); // 正确
// std::string result3 = add("hello", "world"); // 错误，字符串不满足Numerical概念
)");
            
            output->append("\n概念的主要好处:");
            output->append("1. 提供更清晰的编译错误信息");
            output->append("2. Constrain the types of template parameters");
            output->append("3. 改进代码可读性和API设计");
            output->append("4. 支持函数重载基于概念");
        }
    );
    
    // 协程(Coroutines)
    table->addFeature(
        "协程(Coroutines)", 
        "C++ featrue",
        "Simplify asynchronous programming",
        [](QTextEdit* output) {
            output->append("C++20协程 - 代码示例:");
            output->append(R"(
#include <coroutine>
#include <iostream>

struct ReturnObject {
    struct promise_type {
        ReturnObject get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

struct Awaiter {
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        std::thread([h]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            h.resume();
        }).detach();
    }
    void await_resume() {}
};

ReturnObject coroutineFunction() {
    std::cout << "协程开始\n";
    co_await Awaiter{};
    std::cout << "协程恢复执行\n";
}

// 使用
int main() {
    coroutineFunction();
    std::cout << "主函数继续执行\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
}
)");
            
            // output->append("\n协程的主要好处:");
            // output->append("1. 使异步代码看起来像同步代码");
            // output->append("2. 简化状态机实现");
            // output->append("3. 支持生成器和异步IO操作");
        }
    );
    
    // 范围库(Ranges)
    table->addFeature(
        "范围库(Ranges)", 
        "std",
        "Simplify sequence operations",
        [](QTextEdit* output) {
//            output->append("C++20范围库 - 代码示例:");
            output->append(R"(
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // 使用范围库筛选出所有偶数并乘以2
    auto results = numbers 
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * 2; });
    
    // 输出结果
    for (auto v : results) {
        std::cout << v << " "; // 输出: 4 8 12 16 20
    }
}
)");
            
            output->append("\n范围库的主要好处:");
            output->append("1. 支持链式操作");
            output->append("2. 惰性求值，提高性能");
            output->append("3. 更简洁的代码");
            output->append("4. Better readability");
        }
    );
}

void ModernCppWidget::setupCpp23Features(CppFeatureTable* table)
{
    // std::expected
    table->addFeature(
        "std::expected", 
        "std",
        "更好地处理可能失败的操作", 
        [](QTextEdit* output) {
            output->append("C++23 std::expected - 代码示例:");
            output->append(R"(
#include <expected>
#include <string>
#include <iostream>

enum class ErrorCode { FileNotFound, NetworkError, OutOfMemory };

// 返回expected的函数
std::expected<std::string, ErrorCode> readFileContent(const std::string& path) {
    // 检查文件是否存在
    if (path.empty()) {
        return std::unexpected(ErrorCode::FileNotFound);
    }
    
    // 假设我们成功读取了文件
    return "文件内容...";
}

int main() {
    auto result = readFileContent("example.txt");
    
    if (result) {
        std::cout << "成功读取文件: " << *result << std::endl;
    } else {
        if (result.error() == ErrorCode::FileNotFound) {
            std::cout << "错误: 文件未找到" << std::endl;
        }
    }
}
)");
            
            output->append("\nstd::expected的主要好处:");
            output->append("1. Error handling that is lighter than exceptions");
            output->append("2. Enforce the inspection of incorrect conditions");
            output->append("3. 适合需要返回错误信息的函数");
        }
    );
    
    // 多维下标运算符
    table->addFeature(
        "Multi-dimensional subscript operator",
        "C++ feature",
        "Simplify the access to multi-dimensional arrays",
        [](QTextEdit* output) {
            output->append("C++23多维下标运算符 - 代码示例:");
            output->append(R"(
#include <iostream>

class Matrix {
private:
    int data[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

public:
    // C++23支持的多维下标运算符
    auto& operator[](int i, int j) {
        return data[i][j];
    }
    
    const auto& operator[](int i, int j) const {
        return data[i][j];
    }
};

int main() {
    Matrix m;
    std::cout << m[1, 2] << std::endl; // 输出6
    m[0, 0] = 10;
    std::cout << m[0, 0] << std::endl; // 输出10
}
)");
            
            output->append("\n多维下标运算符的好处:");
            output->append("1. 提供更自然的多维数组访问语法");
            output->append("2. 可以在自定义类中实现更直观的接口");
        }
    );
    
    // std::flat_map
    table->addFeature(
        "std::flat_map", 
        "std",
        "更高效的小型映射实现", 
        [](QTextEdit* output) {
            output->append("C++23 std::flat_map - 代码示例:");
            output->append(R"(
#include <flat_map>
#include <string>
#include <iostream>

int main() {
    // 创建一个flat_map
    std::flat_map<std::string, int> scores = {
        {"Alice", 95},
        {"Bob", 87},
        {"Charlie", 92}
    };
    
    // 查询元素
    std::cout << "Bob的分数: " << scores["Bob"] << std::endl;
    
    // 添加新元素
    scores["David"] = 78;
    
    // flat_map特性：内部使用排序数组，小型映射更高效
    // 遍历
    for (const auto& [name, score] : scores) {
        std::cout << name << ": " << score << std::endl;
    }
}
)");
            
            output->append("\nstd::flat_map的主要优势:");
            // output->append("1. 对于小型映射，比std::map更高效");
            // output->append("2. 更好的缓存局部性");
            // output->append("3. 内存占用更少");
            // output->append("4. 提供与std::map相同的接口");
        }
    );
} 
