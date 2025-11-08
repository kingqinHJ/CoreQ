#include "mutexdemowidget.h"
#include <QScrollBar>
#include <QDateTime>
#include <thread>
#include <vector>
#include <mutex>
#include <shared_mutex>

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
/*实现说明

- 运行方式：在后台启动一个“管理线程”，该线程再创建 threadCount 个工作线程，每个线程对同一个 sharedCounter 做 iterationsPerThread 次自增。
- 结果预期：理论上总增量应为 expected = 线程数 × 每线程迭代次数 ，但由于未加锁，多个线程会并发执行“读-改-写”，导致丢增量，实际值通常小于期望值。
- UI安全性：所有界面更新通过 QMetaObject::invokeMethod(..., Qt::QueuedConnection) 回到主线程执行，避免直接从 std::thread 触碰 Qt UI。
- yield 作用：每过一段迭代让出时间片，可减少单线程长时间占用 CPU，提升整体公平性；对结果不产生本质影响。
*/
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

/*实现说明

- 互斥保护：用 std::mutex 配合 std::lock_guard 将 sharedCounter++ 包裹在临界区内，保证“读-改-写”的原子性，消除数据竞争。
- 正确性预期：最终 actual 应严格等于 expected = 线程数 × 每线程迭代次数 ，与无锁版本相比不再丢增量。
- UI线程安全：后台线程完成后通过 QMetaObject::invokeMethod(..., Qt::QueuedConnection) 回到主线程更新控件与日志。
- 捕获列表：工作线程 lambda 使用 [&, this] ，默认按引用捕获局部变量，并显式捕获 this ；避免“无法隐式捕获”错误。
- 让出时间片： yield() 不是必须，仅用于调度公平性；可移除以提升吞吐。
- 取消建议：在“停止”按钮槽里将 m_stopRequested = true ，工作线程循环检查后尽快退出
*/
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
/*
实现说明

- 使用 std::atomic<int> 作为共享计数器，所有线程对其执行原子自增，避免数据竞争。
- 通过后台管理线程创建多个工作线程，UI 不被阻塞；结束后用 QMetaObject::invokeMethod(..., Qt::QueuedConnection) 回到主线程更新控件与日志。
- 在原子演示中， actual 应等于 expected = 线程数 × 每线程迭代次数 ，与无锁版本对比可以看到不再丢增量。
*/
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

/**
 * 读写示例：使用 std::shared_mutex 展示“多读共享、写独占”的并发访问模式。
 * - 多个读线程可同时持有共享锁（shared_lock）读取数据；
 * - 写线程需要获取独占锁（unique_lock）才能修改数据，写期间读会被阻塞；
 * - 相比用 std::mutex 全部独占，shared_mutex 能提升读多写少场景的吞吐。
 * 注意：
 * - UI 控件更新必须回到主线程（通过 QMetaObject::invokeMethod）；
 * - 建议使用 std::atomic<bool> m_stopRequested 来支持取消；
 * - 确保工程启用 C++17（shared_mutex 需要）。
 */
void MutexDemoWidget::startReadersWriters()
{
    // 进入运行状态（UI与状态保护逻辑，建议保留并复用）
    if (m_isRunning) return;
    m_isRunning = true;
    m_stopRequested = false; // 建议类里定义：std::atomic<bool> m_stopRequested{false};
    m_statusLabel->setText("读写演示运行中...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 不确定态：旋转指示
    m_stopBtn->setEnabled(true);
    m_startUnlockedBtn->setEnabled(false);
    m_startMutexBtn->setEnabled(false);
    m_startAtomicBtn->setEnabled(false);
    m_startRWBtn->setEnabled(false);
    m_startDeadlockBtn->setEnabled(false);
    addLogUnsafe("启动：shared_mutex 支持多读共享、写独占。读多写少场景提升吞吐。");

    std::thread([this]{
        // 共享数据与并发原语
        std::vector<int> data;
        data.reserve(1000);
        std::shared_mutex rxMutex;      // 读共享、写独占的读写锁
        std::atomic<int> readCount(0);    // 统计读操作次数
        std::atomic<int> writeCount(0);    // 统计写操作次数

        const int readerCount=6;
        const int writerCount=2;
        const int iterationsPerReader=150000;
        const int iterationsPerWriter=40000;

        std::vector<std::thread> readers;
        std::vector<std::thread> workers;
        readers.reserve(readerCount);
        workers.reserve(writerCount);

        // 创建读线程：使用 shared_lock 允许并发读取
        for(int r=0;r<readerCount;++r)
        {
            readers.emplace_back([&,this]{
                for(int i=0;i<iterationsPerReader;++i)
                {
                    if(m_stopRequested) break;
                    // 读共享锁：多个读者可同时持有
                    std::shared_lock<std::shared_mutex> lock(rxMutex);
                    
                    // 模拟读取：查看大小或最后元素（这里用大小即可）
                    int sizeSnapshot=static_cast<int>(data.size());
                    (void)sizeSnapshot;

                    ++readCount;

                    if(i%20000==0)
                    {
                        std::this_thread::yield();
                    }
                }
            });
        }

        // 创建写线程：使用 unique_lock 独占修改
        for(int w=0;w<writerCount;++w)
        {
            workers.emplace_back([&,this]{
                for(int i=0;i<iterationsPerWriter;++i)
                {
                    if(m_stopRequested) break;
                    // 写独占锁：写期间阻塞读与其他写
                    std::unique_lock<std::shared_mutex> lock(rxMutex);                    
                    data.push_back(i);   // 修改共享数据（示例：追加元素
                    ++writeCount;

                    if(i%5000==0)
                    {
                        std::this_thread::yield();
                    }
                }
            });
        }

        for(auto &t:readers)  t.join();
        for(auto &t:workers)  t.join();
        const int totalReads=readCount.load(std::memory_order_relaxed);
        const int totalWrites=writeCount.load(std::memory_order_relaxed);
        const int finalSize=static_cast<int>(data.size());

        QMetaObject::invokeMethod(this, [this, totalReads, totalWrites, finalSize] {
            m_progressBar->setRange(0, 100);
            m_progressBar->setValue(100);
            m_statusLabel->setText("读写演示完成");
            m_stopBtn->setEnabled(false);
            m_startUnlockedBtn->setEnabled(true);
            m_startMutexBtn->setEnabled(true);
            m_startAtomicBtn->setEnabled(true);
            m_startRWBtn->setEnabled(true);
            m_startDeadlockBtn->setEnabled(true);
            m_isRunning = false;

            addLogUnsafe(QString("结束：读次数=%1，写次数=%2，最终数据量=%3（shared_mutex 保障一致性）")
                         .arg(totalReads).arg(totalWrites).arg(finalSize));
        }, Qt::QueuedConnection);

    }).detach();
}

/**
 * 函数：MutexDemoWidget::startDeadlockDemo
 * 作用：演示“两个互斥锁交叉获取”导致的死锁风险，并展示两种安全解法。
 * 步骤：
 * 1) 设置UI进入运行态，避免重复启动；
 * 2) 使用两个 `std::timed_mutex` 启动两条线程，分别以相反顺序获取锁：
 *    - T1: 先锁 A，再尝试锁 B；
 *    - T2: 先锁 B，再尝试锁 A；
 *    为了避免真的挂死，使用 `try_lock_for` 设置超时，以“超时”来识别潜在死锁；
 * 3) 展示正确解法：
 *    - 解法一：使用 `std::scoped_lock(a, b)` 一次性获取多个互斥锁；
 *    - 解法二（说明在解释中）：固定锁顺序或使用 `std::lock(a, b) + std::adopt_lock`。
 * 4) 回到UI线程更新状态与日志。
 * 说明：
 * - 为了安全演示，这里使用 `std::timed_mutex` + `try_lock_for` 避免真实的无限阻塞；
 * - 所有UI控件更新都通过 `QMetaObject::invokeMethod` 回到主线程；
 * - 建议类中有 `std::atomic<bool> m_stopRequested{false};`，用于停止演示。
 */
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

    std::thread([this]{
        using namespace std::chrono_literals;
        auto postLog=[this](const QString& msg)
        {
            QMetaObject::invokeMethod(this, [this, msg] {
                addLogUnsafe(msg);
            }, Qt::QueuedConnection);
        };

        // 演示死锁风险：两个 timed_mutex，交叉获取，使用 try_lock_for 来“超时检测”
        std::timed_mutex lock_a;
        std::timed_mutex lock_b;

        std::thread t1([&,this]{
            // T1：先A后B
            std::unique_lock<std::timed_mutex> lock_a_guard(lock_a, std::defer_lock);
            if(!lock_a_guard.try_lock_for(500ms))
            {
                postLog("T1：超时等待A锁失败，可能死锁风险。");
                return;
            }
            postLog("T1：成功获取A锁。");
            std::this_thread::sleep_for(100ms);  // 故意制造交叉窗口
            std::unique_lock<std::timed_mutex> lock_b_guard(lock_b, std::defer_lock);
            if(!lock_b_guard.try_lock_for(500ms))
            {
                postLog("T1：超时等待B锁失败，可能死锁风险。");
                return;
            }
            postLog("T1：成功获取 A 与 B（此次未发生死锁）。");
        });

        std::thread t2([&,this]{
            // T2：先B后A
            std::unique_lock<std::timed_mutex> lock_b_guard(lock_b, std::defer_lock);
            if(!lock_b_guard.try_lock_for(500ms))
            {
                postLog("T2：超时等待B锁失败，可能死锁风险。");
                return;
            }
            postLog("T2：成功获取B锁。");
            std::this_thread::sleep_for(100ms);  // 故意制造交叉窗口
            std::unique_lock<std::timed_mutex> lock_a_guard(lock_a, std::defer_lock);
            if(!lock_a_guard.try_lock_for(500ms))
            {
                postLog("T2：超时等待A锁失败，可能死锁风险。");
                return;
            }
            postLog("T2：成功获取 B 与 A（此次未发生死锁）。");
        });

        t1.join();
        t2.join();

        std::mutex safe_a;
        std::mutex safe_b;
        std::thread s1([&,this]{
            std::scoped_lock lock(safe_a, safe_b);
            postLog("S1：成功获取A锁和B锁。");
            std::this_thread::sleep_for(50ms);
        });

        std::thread s2([&,this]{
            std::scoped_lock lock(safe_b, safe_a);
            postLog("S2：成功获取B锁和A锁。");
            std::this_thread::sleep_for(50ms);
        });
        s1.join();
        s2.join();

        QMetaObject::invokeMethod(this, [this] {
            m_progressBar->setRange(0, 100);
            m_progressBar->setValue(100);
            m_statusLabel->setText("死锁演示完成");
            m_stopBtn->setEnabled(false);
            m_startUnlockedBtn->setEnabled(true);
            m_startMutexBtn->setEnabled(true);
            m_startAtomicBtn->setEnabled(true);
            m_startRWBtn->setEnabled(true);
            m_startDeadlockBtn->setEnabled(true);
            m_isRunning = false;

            addLogUnsafe("结束：已演示死锁触发条件与两种解决方案。");
        }, Qt::QueuedConnection);
    }).detach();
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


//QT的线程同步、C++官方的线程同步方式
//QT时间循环、使用信号量进行同步等待，
//C++
