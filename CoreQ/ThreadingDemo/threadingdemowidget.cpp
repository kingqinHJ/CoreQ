#include "threadingdemowidget.h"
#include <QApplication>

ThreadingDemoWidget::ThreadingDemoWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(new QHBoxLayout(this))
    , leftLayout(new QVBoxLayout())
    , titleLabel(new QLabel("C++ 多线程技术演示", this))
    , navigationList(new QListWidget(this))
    , contentStack(new QStackedWidget(this))
    , promiseDemo(nullptr)
    , threadDemo(nullptr)
    , welcomePage(new QWidget(this))
    , splitter(new QSplitter(Qt::Horizontal, this))
{
    setupUI();
    initNavigationList();
    createDemoPages();
    initConnections();
}

ThreadingDemoWidget::~ThreadingDemoWidget()
{
    // 智能指针会自动清理资源
}

void ThreadingDemoWidget::setupUI()
{
    // 设置窗口属性
    setWindowTitle("C++ 多线程技术演示");
    setMinimumSize(800, 600);
    
    // 配置标题
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    padding: 10px;"
        "    background-color: #ecf0f1;"
        "    border-radius: 5px;"
        "    margin-bottom: 10px;"
        "}"
    );
    
    // 配置导航列表
    navigationList->setMaximumWidth(200);
    navigationList->setStyleSheet(
        "QListWidget {"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    background-color: #ffffff;"
        "    selection-background-color: #3498db;"
        "    selection-color: white;"
        "}"
        "QListWidget::item {"
        "    padding: 8px;"
        "    border-bottom: 1px solid #ecf0f1;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #e8f4fd;"
        "}"
    );
    
    // 配置内容区域
    contentStack->setStyleSheet(
        "QStackedWidget {"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    background-color: #ffffff;"
        "}"
    );
    
    // 左侧布局
    leftLayout->addWidget(titleLabel);
    leftLayout->addWidget(navigationList);
    leftLayout->addStretch();
    
    // 创建左侧容器
    QWidget *leftWidget = new QWidget();
    leftWidget->setLayout(leftLayout);
    leftWidget->setMaximumWidth(220);
    
    // 使用分割器
    splitter->addWidget(leftWidget);
    splitter->addWidget(contentStack);
    splitter->setStretchFactor(0, 0);  // 左侧固定
    splitter->setStretchFactor(1, 1);  // 右侧可伸缩
    
    // 主布局
    mainLayout->addWidget(splitter);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    setLayout(mainLayout);
}

void ThreadingDemoWidget::initNavigationList()
{
    // 添加导航项目
    QListWidgetItem *welcomeItem = new QListWidgetItem("欢迎页面");
    welcomeItem->setData(Qt::UserRole, "welcome");
    welcomeItem->setIcon(QIcon(":/icons/home.png")); // 如果有图标资源
    navigationList->addItem(welcomeItem);
    
    QListWidgetItem *promiseItem = new QListWidgetItem("Promise/Future 演示");
    promiseItem->setData(Qt::UserRole, "promise");
    promiseItem->setIcon(QIcon(":/icons/async.png")); // 如果有图标资源
    navigationList->addItem(promiseItem);
    
    // 预留更多演示项目
    QListWidgetItem *threadItem = new QListWidgetItem("std::thread 演示");
    threadItem->setData(Qt::UserRole, "thread");
    navigationList->addItem(threadItem);
    
    QListWidgetItem *mutexItem = new QListWidgetItem("std::mutex 演示");
    mutexItem->setData(Qt::UserRole, "mutex");
    //mutexItem->setEnabled(false); // 暂时禁用，待实现
    navigationList->addItem(mutexItem);
    
    QListWidgetItem *conditionItem = new QListWidgetItem("condition_variable 演示");
    conditionItem->setData(Qt::UserRole, "condition");
    //conditionItem->setEnabled(false); // 暂时禁用，待实现
    navigationList->addItem(conditionItem);
    
    // 默认选中第一项
    navigationList->setCurrentRow(0);
}

void ThreadingDemoWidget::createDemoPages()
{
    // 创建欢迎页面
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomePage);
    
    QLabel *welcomeTitle = new QLabel("欢迎使用 C++ 多线程技术演示", welcomePage);
    welcomeTitle->setAlignment(Qt::AlignCenter);
    welcomeTitle->setStyleSheet(
        "QLabel {"
        "    font-size: 24px;"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    margin: 20px;"
        "}"
    );
    
    QLabel *welcomeText = new QLabel(
        "本演示程序展示了C++11及以后版本中的多线程编程技术：\n\n"
        "• std::promise/std::future - 异步计算和结果获取\n"
        "• std::thread - 基础线程管理\n"
        "• std::mutex - 互斥锁和线程同步\n"
        "• std::condition_variable - 条件变量\n"
        "• 以及更多高级并发技术...\n\n"
        "请从左侧导航选择您想要了解的技术演示。",
        welcomePage
    );
    welcomeText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    welcomeText->setWordWrap(true);
    welcomeText->setStyleSheet(
        "QLabel {"
        "    font-size: 14px;"
        "    line-height: 1.6;"
        "    color: #34495e;"
        "    padding: 20px;"
        "    background-color: #f8f9fa;"
        "    border-radius: 8px;"
        "    border: 1px solid #e9ecef;"
        "}"
    );
    
    QPushButton *quickStartBtn = new QPushButton("快速开始 - Promise演示", welcomePage);
    quickStartBtn->setStyleSheet(
        "QPushButton {"
        "    font-size: 14px;"
        "    padding: 10px 20px;"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
    );
    
    connect(quickStartBtn, &QPushButton::clicked, this, &ThreadingDemoWidget::openPromiseDemo);
    
    welcomeLayout->addWidget(welcomeTitle);
    welcomeLayout->addWidget(welcomeText);
    welcomeLayout->addWidget(quickStartBtn, 0, Qt::AlignCenter);
    welcomeLayout->addStretch();
    
    // 添加页面到堆栈
    contentStack->addWidget(welcomePage);
    
    // 创建Promise演示页面
    promiseDemo = new QPromise(this);
    contentStack->addWidget(promiseDemo);
    
    // 创建std::thread演示页面
    threadDemo = new StdThreadWidget(this);
    contentStack->addWidget(threadDemo);
}

void ThreadingDemoWidget::initConnections()
{
    // 连接导航列表选择变化信号
    connect(navigationList, &QListWidget::currentItemChanged,
            this, &ThreadingDemoWidget::onNavigationSelectionChanged);
}

void ThreadingDemoWidget::onNavigationSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    
    if (!current) {
        return;
    }
    
    QString demoType = current->data(Qt::UserRole).toString();
    
    if (demoType == "welcome") {
        contentStack->setCurrentWidget(welcomePage);
    }
    else if (demoType == "promise") {
        if (promiseDemo) {
            contentStack->setCurrentWidget(promiseDemo);
        }
    }
    else if (demoType == "thread") {
        if (threadDemo) {
            contentStack->setCurrentWidget(threadDemo);
        }
    }
    // 其他演示类型的处理可以在这里添加
}

void ThreadingDemoWidget::openPromiseDemo()
{
    // 切换到Promise演示页面
    for (int i = 0; i < navigationList->count(); ++i) {
        QListWidgetItem *item = navigationList->item(i);
        if (item && item->data(Qt::UserRole).toString() == "promise") {
            navigationList->setCurrentItem(item);
            break;
        }
    }
}