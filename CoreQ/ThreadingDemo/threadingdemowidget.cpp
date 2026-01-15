#include "threadingdemowidget.h"
#include <QApplication>
#include "qtthreadbasicswidget.h"
#include "qtconcurrentwidget.h"
#include "qtproducerconsumerwidget.h"
#include "qtreaderswriterswidget.h"
#include "qtparallelmapwidget.h"
#include "qtpipelinewidget.h"
#include "qtdiningphilosopherswidget.h"

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
        "    border: 1px solid #bdc3c7; /* 保持边框颜色 */"
        "    border-radius: 5px; /* 保持圆角 */"
        "    background-color: #ffffff; /* 保持背景色 */"
        "    outline: none; /* 移除选中时的虚线框，提升美观度 */"
        "}"

        "QListWidget::item {"
        "    padding: 10px 12px; /* 增加垂直和水平内边距，提升视觉舒适度 */"
        "    border-bottom: 1px solid #ecf0f1; /* 保持分隔线 */"
        "    color: #333333; /* 默认文字颜色，比纯黑更柔和 */"
        "}"

        "QListWidget::item:hover {"
        "    background-color: #f0f0f0; /* 悬停背景色改为浅灰色，更柔和 */"
        "    color: #333333; /* 悬停时文字颜色保持默认 */"
        "}"

        "QListWidget::item:selected {"
        "    background-color: #3498db; /* 选中项背景色保持蓝色 */"
        "    color: #ffffff; /* **优化：选中项文字颜色改为白色，提高可读性** */"
        "}"

        // 可选：如果希望选中项在悬停时有不同效果，可以添加
        "QListWidget::item:selected:hover {"
        "    background-color: #2980b9; /* 选中项在悬停时，背景色稍微变深 */"
        "    color: #ffffff;"
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
    // 清空列表
    navigationList->clear();

    // ==========================================
    // C++11 Std 线程库
    // ==========================================
    QListWidgetItem *headerStd = new QListWidgetItem("=== C++11 Std 线程库 ===");
    headerStd->setFlags(Qt::NoItemFlags); // 不可选中
    headerStd->setBackground(QColor("#ecf0f1"));
    headerStd->setForeground(QColor("#7f8c8d"));
    headerStd->setTextAlignment(Qt::AlignCenter);
    navigationList->addItem(headerStd);

    // 添加原有导航项目
    QListWidgetItem *welcomeItem = new QListWidgetItem("欢迎页面");
    welcomeItem->setData(Qt::UserRole, "welcome");
    welcomeItem->setIcon(QIcon(":/icons/home.png")); 
    navigationList->addItem(welcomeItem);
    
    QListWidgetItem *promiseItem = new QListWidgetItem("Promise/Future 演示");
    promiseItem->setData(Qt::UserRole, "promise");
    navigationList->addItem(promiseItem);
    
    QListWidgetItem *threadItem = new QListWidgetItem("std::thread 演示");
    threadItem->setData(Qt::UserRole, "thread");
    navigationList->addItem(threadItem);
    
    QListWidgetItem *mutexItem = new QListWidgetItem("std::mutex 演示");
    mutexItem->setData(Qt::UserRole, "mutex");
    navigationList->addItem(mutexItem);
    
    QListWidgetItem *conditionItem = new QListWidgetItem("condition_variable 演示");
    conditionItem->setData(Qt::UserRole, "condition");
    navigationList->addItem(conditionItem);

    // ==========================================
    // Qt 线程基础
    // ==========================================
    QListWidgetItem *headerQtBasic = new QListWidgetItem("=== Qt 线程基础 ===");
    headerQtBasic->setFlags(Qt::NoItemFlags);
    headerQtBasic->setBackground(QColor("#ecf0f1"));
    headerQtBasic->setForeground(QColor("#7f8c8d"));
    headerQtBasic->setTextAlignment(Qt::AlignCenter);
    navigationList->addItem(headerQtBasic);

    QListWidgetItem *basicsItem = new QListWidgetItem("线程管理与生命周期");
    basicsItem->setData(Qt::UserRole, "qt_basics");
    navigationList->addItem(basicsItem);

    QListWidgetItem *concurrentItem = new QListWidgetItem("高级并发 (QtConcurrent)");
    concurrentItem->setData(Qt::UserRole, "qt_concurrent");
    navigationList->addItem(concurrentItem);

    // ==========================================
    // 经典并发模型
    // ==========================================
    QListWidgetItem *headerModels = new QListWidgetItem("=== 经典并发模型 ===");
    headerModels->setFlags(Qt::NoItemFlags);
    headerModels->setBackground(QColor("#ecf0f1"));
    headerModels->setForeground(QColor("#7f8c8d"));
    headerModels->setTextAlignment(Qt::AlignCenter);
    navigationList->addItem(headerModels);

    QListWidgetItem *pcItem = new QListWidgetItem("生产者-消费者模型");
    pcItem->setData(Qt::UserRole, "qt_pc");
    navigationList->addItem(pcItem);

    QListWidgetItem *rwItem = new QListWidgetItem("读写者模型");
    rwItem->setData(Qt::UserRole, "qt_rw");
    navigationList->addItem(rwItem);

    QListWidgetItem *mapItem = new QListWidgetItem("并行计算 (Map-Reduce)");
    mapItem->setData(Qt::UserRole, "qt_map");
    navigationList->addItem(mapItem);
    
    QListWidgetItem *pipelineItem = new QListWidgetItem("流水线模型");
    pipelineItem->setData(Qt::UserRole, "qt_pipeline");
    navigationList->addItem(pipelineItem);

    QListWidgetItem *diningItem = new QListWidgetItem("哲学家就餐 (死锁)");
    diningItem->setData(Qt::UserRole, "qt_dining");
    navigationList->addItem(diningItem);
    
    // 默认选中第一项
    navigationList->setCurrentRow(1); // 选中欢迎页面 (index 0 是header)
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

    // 创建std::mutex演示页面
    mutexDemo = new MutexDemoWidget(this);
    contentStack->addWidget(mutexDemo);

    // 创建std::condition_variable演示页面
    cvDemo = new ConditionVariableWidget(this);
    contentStack->addWidget(cvDemo);

    // ==========================================
    // 新增 Qt 多线程演示页面
    // ==========================================
    
    // 5. Qt 线程基础
    contentStack->addWidget(new QtThreadBasicsWidget(this));
    
    // 6. Qt Concurrent
    contentStack->addWidget(new QtConcurrentWidget(this));
    
    // 7. 生产者-消费者
    contentStack->addWidget(new QtProducerConsumerWidget(this));
    
    // 8. 读写锁
    contentStack->addWidget(new QtReadersWritersWidget(this));
    
    // 9. 并行 Map
    contentStack->addWidget(new QtParallelMapWidget(this));
    
    // 10. 流水线
    contentStack->addWidget(new QtPipelineWidget(this));
    
    // 11. 哲学家就餐
    contentStack->addWidget(new QtDiningPhilosophersWidget(this));
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
    else if (demoType == "mutex") {
        if (mutexDemo) {
            contentStack->setCurrentWidget(mutexDemo);
        }
    }
    else if (demoType == "condition") {
        if (cvDemo) {
            contentStack->setCurrentWidget(cvDemo);
        }
    }
    else if (demoType == "qt_basics") {
        contentStack->setCurrentIndex(5);
    }
    else if (demoType == "qt_concurrent") {
        contentStack->setCurrentIndex(6);
    }
    else if (demoType == "qt_pc") {
        contentStack->setCurrentIndex(7);
    }
    else if (demoType == "qt_rw") {
        contentStack->setCurrentIndex(8);
    }
    else if (demoType == "qt_map") {
        contentStack->setCurrentIndex(9);
    }
    else if (demoType == "qt_pipeline") {
        contentStack->setCurrentIndex(10);
    }
    else if (demoType == "qt_dining") {
        contentStack->setCurrentIndex(11);
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