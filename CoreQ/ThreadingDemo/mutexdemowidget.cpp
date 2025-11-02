#include "mutexdemowidget.h"
#include <QScrollBar>
#include <QDateTime>
#include <thread>
#include <vector>
#include <mutex>

MutexDemoWidget::MutexDemoWidget(QWidget* parent)
    : QWidget(parent)
{
    initUI();
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MutexDemoWidget::updateUI);
    m_updateTimer->start(100);
}

MutexDemoWidget::~MutexDemoWidget() {}

void MutexDemoWidget::initUI()
{
    setWindowTitle("std::mutex 演示");
    setMinimumSize(800, 600);

    auto* mainLayout = new QVBoxLayout(this);

    // 计数演示组
    m_counterGroup = new QGroupBox("共享计数器演示", this);
    auto* counterLayout = new QHBoxLayout(m_counterGroup);

    counterLayout->addWidget(new QLabel("线程数量:"));
    m_threadCount = new QSpinBox();
    m_threadCount->setRange(1, 64);
    m_threadCount->setValue(8);
    counterLayout->addWidget(m_threadCount);

    counterLayout->addWidget(new QLabel("每线程迭代次数:"));
    m_iterations = new QSpinBox();
    m_iterations->setRange(100, 1000000);
    m_iterations->setValue(10000);
    counterLayout->addWidget(m_iterations);

    m_startUnlockedBtn = new QPushButton("不加锁执行");
    m_startMutexBtn   = new QPushButton("使用std::mutex执行");
    m_startAtomicBtn  = new QPushButton("使用std::atomic执行");
    counterLayout->addWidget(m_startUnlockedBtn);
    counterLayout->addWidget(m_startMutexBtn);
    counterLayout->addWidget(m_startAtomicBtn);
    counterLayout->addStretch();

    // 读者-写者演示组
    m_rwGroup = new QGroupBox("读者-写者演示", this);
    auto* rwLayout = new QHBoxLayout(m_rwGroup);
    rwLayout->addWidget(new QLabel("读线程:"));
    m_readerThreads = new QSpinBox();
    m_readerThreads->setRange(1, 64);
    m_readerThreads->setValue(6);
    rwLayout->addWidget(m_readerThreads);

    rwLayout->addWidget(new QLabel("写线程:"));
    m_writerThreads = new QSpinBox();
    m_writerThreads->setRange(1, 16);
    m_writerThreads->setValue(2);
    rwLayout->addWidget(m_writerThreads);

    m_startRWBtn = new QPushButton("启动读者-写者测试");
    rwLayout->addWidget(m_startRWBtn);
    rwLayout->addStretch();

    // 死锁演示组
    m_deadlockGroup = new QGroupBox("死锁与避免", this);
    auto* deadlockLayout = new QHBoxLayout(m_deadlockGroup);
    m_startDeadlockBtn = new QPushButton("触发死锁/避免");
    deadlockLayout->addWidget(m_startDeadlockBtn);
    deadlockLayout->addStretch();

    // 控制区
    auto* controlLayout = new QHBoxLayout();
    m_stopBtn = new QPushButton("停止");
    m_stopBtn->setEnabled(false);
    m_clearBtn = new QPushButton("清空日志");
    controlLayout->addWidget(m_stopBtn);
    controlLayout->addWidget(m_clearBtn);
    controlLayout->addStretch();

    // 状态与日志
    auto* statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("就绪");
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    statusLayout->addWidget(new QLabel("状态:"));
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addWidget(m_progressBar);

    m_logDisplay = new QTextEdit();
    m_logDisplay->setReadOnly(true);
    m_logDisplay->setMaximumHeight(220);

    // 主布局拼装
    mainLayout->addWidget(m_counterGroup);
    mainLayout->addWidget(m_rwGroup);
    mainLayout->addWidget(m_deadlockGroup);
    mainLayout->addLayout(controlLayout);
    mainLayout->addLayout(statusLayout);
    mainLayout->addWidget(m_logDisplay);

    // 连接信号槽（占位）
    connect(m_startUnlockedBtn, &QPushButton::clicked, this, &MutexDemoWidget::startUnlockedCount);
    connect(m_startMutexBtn,   &QPushButton::clicked, this, &MutexDemoWidget::startMutexCount);
    connect(m_startAtomicBtn,  &QPushButton::clicked, this, &MutexDemoWidget::startAtomicCount);
    connect(m_startRWBtn,      &QPushButton::clicked, this, &MutexDemoWidget::startReadersWriters);
    connect(m_startDeadlockBtn,&QPushButton::clicked, this, &MutexDemoWidget::startDeadlockDemo);
    connect(m_stopBtn,         &QPushButton::clicked, this, &MutexDemoWidget::stopAll);
    connect(m_clearBtn,        &QPushButton::clicked, this, &MutexDemoWidget::clearLog);

    // 简单样式
    setStyleSheet(R"(
        QGroupBox { font-weight: bold; border: 2px solid #cccccc; border-radius: 5px; margin-top: 1ex; padding-top: 10px; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
        QPushButton { background-color: #4CAF50; border: none; color: white; padding: 8px 16px; border-radius: 4px; font-weight: bold; }
        QPushButton:hover { background-color: #45a049; }
        QPushButton:pressed { background-color: #3d8b40; }
        QPushButton:disabled { background-color: #cccccc; color: #666666; }
    )");
}

// 占位：不执行真实并发，仅调整UI并输出日志
void MutexDemoWidget::startUnlockedCount()
{
    if (m_isRunning) return;
    m_isRunning = true;
    m_statusLabel->setText("无锁计数演示（占位）运行中...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0);
    m_stopBtn->setEnabled(true);
    m_startUnlockedBtn->setEnabled(false);
    m_startMutexBtn->setEnabled(false);
    m_startAtomicBtn->setEnabled(false);
    m_startRWBtn->setEnabled(false);
    m_startDeadlockBtn->setEnabled(false);
    addLogUnsafe("启动：无锁共享计数（占位）。后续请在此处加入并发逻辑。");

    std::thread([this]{
        const int threadCount=8; //工资线程数
        const int itertionsPerThread=200000;
        const int expected=threadCount*itertionsPerThread;

        int sharedCounter=0;  // 故意未加锁，制造数据竞争

        std::vector<std::thread> workers;
        workers.reserve(threadCount);

        for(int t=0;t<threadCount;++t)
        {
            workers.emplace_back([this,&sharedCounter,itertionsPerThread]{// 读-改-写，存在数据竞争
                for(int i=0;i<itertionsPerThread;++i)
                {
                    if(m_stopRequested) // 支持取消
                        break;

                    sharedCounter++;  
                    if((i%10000)==0)
                    {
                        std::this_thread::yield();  // 让出时间片，提升公平性
                    }    
                }

            });
        }

        for(auto &th:workers){
            th.join();
        }

        const int actual=sharedCounter;

        // 回到UI线程安全更新界面
        QMetaObject::invokeMethod(this, [this, expected, actual] {
            // 退出运行状态（统一复位）
            m_progressBar->setRange(0, 100);
            m_progressBar->setValue(100);
            m_statusLabel->setText("无锁计数演示完成");
            m_stopBtn->setEnabled(false);
            m_startUnlockedBtn->setEnabled(true);
            m_startMutexBtn->setEnabled(true);
            m_startAtomicBtn->setEnabled(true);
            m_startRWBtn->setEnabled(true);
            m_startDeadlockBtn->setEnabled(true);
            m_isRunning = false;

            addLogUnsafe(QString("结束：期望=%1，实际=%2，差异=%3")
                         .arg(expected).arg(actual).arg(expected - actual));
        }, Qt::QueuedConnection);
    }).detach();
}

void MutexDemoWidget::startMutexCount()
{
    // 进入运行状态（保留并复用这段UI/状态保护逻辑）
    if (m_isRunning) return;
    m_isRunning = true;
    m_stopRequested = false; // 建议定义为 std::atomic<bool> m_stopRequested{false};
    m_statusLabel->setText("互斥锁计数演示运行中...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 不确定态：旋转指示
    m_stopBtn->setEnabled(true);
    m_startUnlockedBtn->setEnabled(false);
    m_startMutexBtn->setEnabled(false);
    m_startAtomicBtn->setEnabled(false);
    m_startRWBtn->setEnabled(false);
    m_startDeadlockBtn->setEnabled(false);
    addLogUnsafe("启动：互斥锁保护下的共享计数。锁保证正确性，但有开销。");

    std::thread([this]{
        const int threadCount=8;
        const int iterationPerThread=200000;
        const int expected=threadCount*iterationPerThread;

        int sharedCounter=0;
        std::mutex counterMutex;
        
        std::vector<std::thread> workers;
        workers.reserve(threadCount);

        for(int t=0;t<threadCount;++t){
            workers.emplace_back([&,this]{
                for(int i=0;i<iterationPerThread;++i){
                    if(m_stopRequested)
                        break;
                    {
                        std::lock_guard<std::mutex> lk(counterMutex);
                            ++sharedCounter;
                    }

                    if((i%10000)==0){
                        std::this_thread::yield();
                    }
                }
            });
        }

        for(auto &th:workers)
                th.join();
        const int actual=sharedCounter;
        QMetaObject::invokeMethod(this,[this,expected,actual]{
            m_progressBar->setRange(0, 100);
            m_progressBar->setValue(100);
            m_statusLabel->setText("互斥锁计数演示完成");
            m_stopBtn->setEnabled(false);
            m_startUnlockedBtn->setEnabled(true);
            m_startMutexBtn->setEnabled(true);
            m_startAtomicBtn->setEnabled(true);
            m_startRWBtn->setEnabled(true);
            m_startDeadlockBtn->setEnabled(true);
            m_isRunning = false;
            addLogUnsafe(QString("结束：期望=%1，实际=%2（锁保证正确性）").arg(expected).arg(actual));
        },Qt::QueuedConnection);
    }).detach();
}

//使用 std::atomic（非同步，只是安全）
void MutexDemoWidget::startAtomicCount()
{
    // 进入运行状态（建议保留并复用）
    if (m_isRunning) return;
    m_isRunning = true;
    m_stopRequested = false; // 建议定义为 std::atomic<bool> m_stopRequested{false};
    m_statusLabel->setText("原子计数演示运行中...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 不确定态：旋转指示
    m_stopBtn->setEnabled(true);
    m_startUnlockedBtn->setEnabled(false);
    m_startMutexBtn->setEnabled(false);
    m_startAtomicBtn->setEnabled(false);
    m_startRWBtn->setEnabled(false);
    m_startDeadlockBtn->setEnabled(false);
    addLogUnsafe("启动：std::atomic 保护下的共享计数。无锁但无数据竞争。");

    // 后台管理线程，避免阻塞UI线程
    std::thread([this] {
        const int threadCount = 8;
        const int iterationsPerThread = 200000;
        const int expected = threadCount * iterationsPerThread;

        std::atomic<int> sharedCounter(0);
        std::vector<std::thread> workers;
        workers.reserve(threadCount);
        for(int t=0;t<threadCount;++t)
        {
            workers.emplace_back([&,this]{
                for(int i=0;i<iterationsPerThread;++i)
                {
                    if(m_stopRequested) break;
                    //方式一：递增运算符
                    ++sharedCounter;
                    //sharedCounter.fetch_add(1,std::memory_order_relaxed);
                    if(i%10000==0)
                    {
                        std::this_thread::yield();
                    }
                }
            });
        }

        for(auto &th:workers)
            th.join();

        const int actual=sharedCounter.load(std::memory_order_relaxed);

        QMetaObject::invokeMethod(this,[this,expected,actual]{
            m_progressBar->setRange(0, 100);
            m_progressBar->setValue(100);
            m_statusLabel->setText("原子计数演示完成");
            m_stopBtn->setEnabled(false);
            m_startUnlockedBtn->setEnabled(true);
            m_startMutexBtn->setEnabled(true);
            m_startAtomicBtn->setEnabled(true);
            m_startRWBtn->setEnabled(true);
            m_startDeadlockBtn->setEnabled(true);
            m_isRunning = false;

            addLogUnsafe(QString("结束：期望=%1，实际=%2（原子操作保证正确性）")
                         .arg(expected).arg(actual));
        }, Qt::QueuedConnection);
    }).detach();

}

void MutexDemoWidget::startReadersWriters()
{
    if (m_isRunning) return;
    m_isRunning = true;
    m_statusLabel->setText("读者-写者演示（占位）运行中...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0);
    m_stopBtn->setEnabled(true);
    m_startUnlockedBtn->setEnabled(false);
    m_startMutexBtn->setEnabled(false);
    m_startAtomicBtn->setEnabled(false);
    m_startRWBtn->setEnabled(false);
    m_startDeadlockBtn->setEnabled(false);
    addLogUnsafe("启动：读者-写者演示（占位）。");
}

void MutexDemoWidget::startDeadlockDemo()
{
    if (m_isRunning) return;
    m_isRunning = true;
    m_statusLabel->setText("死锁/避免演示（占位）运行中...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0);
    m_stopBtn->setEnabled(true);
    m_startUnlockedBtn->setEnabled(false);
    m_startMutexBtn->setEnabled(false);
    m_startAtomicBtn->setEnabled(false);
    m_startRWBtn->setEnabled(false);
    m_startDeadlockBtn->setEnabled(false);
    addLogUnsafe("启动：死锁/避免演示（占位）。");
}

void MutexDemoWidget::stopAll()
{
    if (!m_isRunning) return;
    m_isRunning = false;
    m_progressBar->setVisible(false);
    m_statusLabel->setText("已停止");
    m_stopBtn->setEnabled(false);
    m_startUnlockedBtn->setEnabled(true);
    m_startMutexBtn->setEnabled(true);
    m_startAtomicBtn->setEnabled(true);
    m_startRWBtn->setEnabled(true);
    m_startDeadlockBtn->setEnabled(true);
    addLogUnsafe("停止：所有占位演示。");
}

void MutexDemoWidget::clearLog()
{
    m_logDisplay->clear();
}

void MutexDemoWidget::updateUI()
{
    if (!m_pendingLogs.isEmpty()) {
        m_logDisplay->append(m_pendingLogs);
        m_pendingLogs.clear();
        QScrollBar* sb = m_logDisplay->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}

void MutexDemoWidget::addLogUnsafe(const QString& msg)
{
    const QString ts = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    m_pendingLogs += QString("[%1] %2\n").arg(ts, msg);
}
