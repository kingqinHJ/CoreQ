#include "stdthreadwidget.h"
#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QScrollBar>
#include <algorithm>
#include <random>

StdThreadWidget::StdThreadWidget(QWidget *parent)
    : QWidget(parent)
    , m_stopFlag(false)
    , m_completedTasks(0)
    , m_totalTasks(0)
    , m_isRunning(false)
{
    initUI();
    
    // 初始化UI更新定时器
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &StdThreadWidget::updateUI);
    m_updateTimer->start(100); // 每100ms更新一次UI
}

StdThreadWidget::~StdThreadWidget()
{
    // 确保所有线程正确清理
    m_stopFlag = true;
    joinAllThreads();
}

void StdThreadWidget::initUI()
{
    setWindowTitle("std::thread 演示");
    setMinimumSize(800, 600);
    
    auto* mainLayout = new QVBoxLayout(this);
    
    // 单线程演示组
    m_singleThreadGroup = new QGroupBox("单线程演示", this);
    auto* singleLayout = new QHBoxLayout(m_singleThreadGroup);
    
    singleLayout->addWidget(new QLabel("工作时间(秒):"));
    m_singleWorkTime = new QSpinBox();
    m_singleWorkTime->setRange(1, 10);
    m_singleWorkTime->setValue(3);
    singleLayout->addWidget(m_singleWorkTime);
    
    m_startSingleBtn = new QPushButton("启动单线程任务");
    singleLayout->addWidget(m_startSingleBtn);
    singleLayout->addStretch();
    
    // 多线程演示组
    m_multiThreadGroup = new QGroupBox("多线程演示", this);
    auto* multiLayout = new QHBoxLayout(m_multiThreadGroup);
    
    multiLayout->addWidget(new QLabel("线程数量:"));
    m_threadCount = new QSpinBox();
    m_threadCount->setRange(2, 8);
    m_threadCount->setValue(4);
    multiLayout->addWidget(m_threadCount);
    
    multiLayout->addWidget(new QLabel("迭代次数:"));
    m_iterations = new QSpinBox();
    m_iterations->setRange(100, 10000);
    m_iterations->setValue(1000);
    multiLayout->addWidget(m_iterations);
    
    m_startMultiBtn = new QPushButton("启动多线程任务");
    multiLayout->addWidget(m_startMultiBtn);
    multiLayout->addStretch();
    
    // 控制按钮
    auto* controlLayout = new QHBoxLayout();
    m_stopBtn = new QPushButton("停止所有线程");
    m_stopBtn->setEnabled(false);
    m_clearBtn = new QPushButton("清空日志");
    
    controlLayout->addWidget(m_stopBtn);
    controlLayout->addWidget(m_clearBtn);
    controlLayout->addStretch();
    
    // 状态显示
    auto* statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("就绪");
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    
    statusLayout->addWidget(new QLabel("状态:"));
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addWidget(m_progressBar);
    
    // 日志显示组
    m_logGroup = new QGroupBox("执行日志", this);
    auto* logLayout = new QVBoxLayout(m_logGroup);
    
    m_logDisplay = new QTextEdit();
    m_logDisplay->setReadOnly(true);
    m_logDisplay->setMaximumHeight(200);
    logLayout->addWidget(m_logDisplay);
    
    // 添加到主布局
    mainLayout->addWidget(m_singleThreadGroup);
    mainLayout->addWidget(m_multiThreadGroup);
    mainLayout->addLayout(controlLayout);
    mainLayout->addLayout(statusLayout);
    mainLayout->addWidget(m_logGroup);
    
    // 连接信号槽
    connect(m_startSingleBtn, &QPushButton::clicked, this, &StdThreadWidget::startSingleThread);
    connect(m_startMultiBtn, &QPushButton::clicked, this, &StdThreadWidget::startMultipleThreads);
    connect(m_stopBtn, &QPushButton::clicked, this, &StdThreadWidget::stopAllThreads);
    connect(m_clearBtn, &QPushButton::clicked, this, &StdThreadWidget::clearLog);
    
    // 设置样式
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QPushButton {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
        }
    )");
}

void StdThreadWidget::startSingleThread()
{
    if (m_isRunning) {
        QMessageBox::warning(this, "警告", "已有线程在运行中，请先停止！");
        return;
    }
    
    m_isRunning = true;
    m_stopFlag = false;
    m_completedTasks = 0;
    m_totalTasks = 1;
    
    // 更新UI状态
    m_startSingleBtn->setEnabled(false);
    m_startMultiBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 不确定进度
    m_statusLabel->setText("单线程运行中...");
    
    addLogSafe("=== 启动单线程演示 ===");
    
    int workTime = m_singleWorkTime->value();
    
    // 创建并启动线程，线程的启动在构造期，不需要“start”。 join / detach 只决定主线程是否等待它或是否分离资源，并不影响是否开始执行。
    m_threads.emplace_back(&StdThreadWidget::singleThreadWork, this, 1, workTime);
    
    addLogSafe(QString("创建线程 ID: %1, 工作时间: %2 秒")
               .arg(1).arg(workTime));
}

void StdThreadWidget::startMultipleThreads()
{
    if (m_isRunning) {
        QMessageBox::warning(this, "警告", "已有线程在运行中，请先停止！");
        return;
    }
    
    m_isRunning = true;
    m_stopFlag = false;
    m_completedTasks = 0;
    
    int threadCount = m_threadCount->value();
    int iterations = m_iterations->value();
    m_totalTasks = threadCount;
    
    // 更新UI状态
    m_startSingleBtn->setEnabled(false);
    m_startMultiBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, threadCount);
    m_progressBar->setValue(0);
    m_statusLabel->setText(QString("多线程运行中... (0/%1)").arg(threadCount));
    
    addLogSafe("=== 启动多线程演示 ===");
    addLogSafe(QString("创建 %1 个线程，每个线程执行 %2 次迭代")
               .arg(threadCount).arg(iterations));
    
    // 创建多个线程
    for (int i = 0; i < threadCount; ++i) {
        m_threads.emplace_back(&StdThreadWidget::multiThreadWork, this, i + 1, iterations);
        addLogSafe(QString("创建线程 %1").arg(i + 1));
    }
}

void StdThreadWidget::stopAllThreads()
{
    if (!m_isRunning) {
        return;
    }
    
    addLogSafe("=== 停止所有线程 ===");
    m_stopFlag = true;
    m_statusLabel->setText("正在停止线程...");
    
    // 等待所有线程完成
    joinAllThreads();
    
    // 重置UI状态
    m_isRunning = false;
    m_startSingleBtn->setEnabled(true);
    m_startMultiBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    m_progressBar->setVisible(false);
    m_statusLabel->setText("已停止");
    
    addLogSafe("所有线程已停止");
}

void StdThreadWidget::clearLog()
{
    m_logDisplay->clear();
}

void StdThreadWidget::updateUI()
{
    // 线程安全地更新UI
    std::lock_guard<std::mutex> lock(m_uiMutex);
    
    if (!m_pendingLogs.isEmpty()) {
        m_logDisplay->append(m_pendingLogs);
        m_pendingLogs.clear();
        
        // 自动滚动到底部
        QScrollBar* scrollBar = m_logDisplay->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
    
    // 更新进度
    if (m_isRunning && m_totalTasks > 1) {
        int completed = m_completedTasks.load();
        m_progressBar->setValue(completed);
        m_statusLabel->setText(QString("多线程运行中... (%1/%2)")
                              .arg(completed).arg(m_totalTasks.load()));
        
        // 检查是否所有任务完成
        if (completed >= m_totalTasks) {
            stopAllThreads();
        }
    }
}

void StdThreadWidget::singleThreadWork(int threadId, int workTime)
{
    addLogSafe(QString("[线程 %1] 开始执行，预计运行 %2 秒")
               .arg(threadId).arg(workTime));
    
    auto startTime = std::chrono::steady_clock::now();
    
    // 模拟工作负载
    for (int i = 0; i < workTime * 10 && !m_stopFlag; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if (i % 10 == 0) {
            addLogSafe(QString("[线程 %1] 进度: %2%")
                       .arg(threadId).arg((i + 1) * 100 / (workTime * 10)));
        }
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    if (m_stopFlag) {
        addLogSafe(QString("[线程 %1] 被中断停止，运行时间: %2 ms")
                   .arg(threadId).arg(duration.count()));
    } else {
        addLogSafe(QString("[线程 %1] 正常完成，运行时间: %2 ms")
                   .arg(threadId).arg(duration.count()));
    }
    
    m_completedTasks++;
}

void StdThreadWidget::multiThreadWork(int threadId, int iterations)
{
    addLogSafe(QString("[线程 %1] 开始执行 %2 次迭代")
               .arg(threadId).arg(iterations));
    
    auto startTime = std::chrono::steady_clock::now();
    
    // 模拟计算密集型任务
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    double result = 0.0;
    for (int i = 0; i < iterations && !m_stopFlag; ++i) {
        // 模拟一些计算
        result += std::sin(dis(gen)) * std::cos(dis(gen));
        
        // 每完成10%报告一次进度
        if (i % (iterations / 10) == 0 && i > 0) {
            addLogSafe(QString("[线程 %1] 进度: %2% (结果: %3)")
                       .arg(threadId)
                       .arg(i * 100 / iterations)
                       .arg(result, 0, 'f', 2));
        }
        
        // 偶尔让出CPU时间
        if (i % 100 == 0) {
            std::this_thread::yield();
        }
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    if (m_stopFlag) {
        addLogSafe(QString("[线程 %1] 被中断停止，运行时间: %2 ms")
                   .arg(threadId).arg(duration.count()));
    } else {
        addLogSafe(QString("[线程 %1] 完成所有迭代，最终结果: %2，运行时间: %3 ms")
                   .arg(threadId).arg(result, 0, 'f', 2).arg(duration.count()));
    }
    
    m_completedTasks++;
}

void StdThreadWidget::addLogSafe(const QString& message)
{
    std::lock_guard<std::mutex> lock(m_uiMutex);
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    m_pendingLogs += QString("[%1] %2\n").arg(timestamp, message);
}

void StdThreadWidget::joinAllThreads()
{
    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    m_threads.clear();
}
