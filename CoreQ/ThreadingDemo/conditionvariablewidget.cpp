#include "conditionvariablewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <chrono>

ConditionVariableWidget::ConditionVariableWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    
    m_uiTimer = new QTimer(this);
    connect(m_uiTimer, &QTimer::timeout, this, &ConditionVariableWidget::updateUI);
    m_uiTimer->start(100); // 10Hz update rate
}

ConditionVariableWidget::~ConditionVariableWidget()
{
    stopAll();
}

void ConditionVariableWidget::initUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    
    // Control Group
    auto *controlGroup = new QGroupBox("生产者-消费者控制", this);
    auto *controlLayout = new QHBoxLayout(controlGroup);
    
    controlLayout->addWidget(new QLabel("生产间隔(ms):"));
    m_producerSpeed = new QSpinBox();
    m_producerSpeed->setRange(10, 2000);
    m_producerSpeed->setValue(500);
    controlLayout->addWidget(m_producerSpeed);
    
    controlLayout->addWidget(new QLabel("消费间隔(ms):"));
    m_consumerSpeed = new QSpinBox();
    m_consumerSpeed->setRange(10, 2000);
    m_consumerSpeed->setValue(1000);
    controlLayout->addWidget(m_consumerSpeed);
    
    controlLayout->addWidget(new QLabel("消费者数量:"));
    m_consumerCount = new QSpinBox();
    m_consumerCount->setRange(1, 5);
    m_consumerCount->setValue(2);
    controlLayout->addWidget(m_consumerCount);
    
    m_startBtn = new QPushButton("开始");
    connect(m_startBtn, &QPushButton::clicked, this, &ConditionVariableWidget::startProducerConsumer);
    controlLayout->addWidget(m_startBtn);
    
    m_stopBtn = new QPushButton("停止");
    m_stopBtn->setEnabled(false);
    connect(m_stopBtn, &QPushButton::clicked, this, &ConditionVariableWidget::stopAll);
    controlLayout->addWidget(m_stopBtn);
    
    controlLayout->addStretch();
    
    // Status Group
    auto *statusGroup = new QGroupBox("缓冲区状态 (Max: 10)", this);
    auto *statusLayout = new QVBoxLayout(statusGroup);
    
    m_bufferBar = new QProgressBar();
    m_bufferBar->setRange(0, 10);
    m_bufferBar->setValue(0);
    m_bufferBar->setFormat("%v / 10");
    statusLayout->addWidget(m_bufferBar);
    
    m_statusLabel = new QLabel("就绪");
    statusLayout->addWidget(m_statusLabel);
    
    // Log Area
    m_logDisplay = new QTextEdit();
    m_logDisplay->setReadOnly(true);
    
    auto *clearBtn = new QPushButton("清空日志");
    connect(clearBtn, &QPushButton::clicked, this, &ConditionVariableWidget::clearLog);
    
    mainLayout->addWidget(controlGroup);
    mainLayout->addWidget(statusGroup);
    mainLayout->addWidget(new QLabel("运行日志:"));
    mainLayout->addWidget(m_logDisplay);
    mainLayout->addWidget(clearBtn);
}

void ConditionVariableWidget::startProducerConsumer()
{
    if (m_running) return;
    
    m_running = true;
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_producerSpeed->setEnabled(false);
    m_consumerSpeed->setEnabled(false);
    m_consumerCount->setEnabled(false);
    
    // Start Producer
    m_threads.emplace_back(&ConditionVariableWidget::producerThread, this);
    
    // Start Consumers
    int consumers = m_consumerCount->value();
    for (int i = 0; i < consumers; ++i) {
        m_threads.emplace_back(&ConditionVariableWidget::consumerThread, this, i + 1);
    }
    
    logMessage("系统启动：1个生产者, " + QString::number(consumers) + "个消费者");
}

void ConditionVariableWidget::stopAll()
{
    if (!m_running) return;
    
    m_running = false;
    
    // 唤醒所有等待中的线程，让它们有机会检查 m_running 并退出
    m_cv_not_empty.notify_all();
    m_cv_not_full.notify_all();
    
    for (auto &t : m_threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    m_threads.clear();
    
    // 清空缓冲区
    std::lock_guard<std::mutex> lock(m_mutex);
    std::queue<int> empty;
    std::swap(m_buffer, empty);
    
    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    m_producerSpeed->setEnabled(true);
    m_consumerSpeed->setEnabled(true);
    m_consumerCount->setEnabled(true);
    
    m_bufferBar->setValue(0);
    m_statusLabel->setText("已停止");
    logMessage("系统已停止");
}

void ConditionVariableWidget::producerThread()
{
    while (m_running) {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        // 等待缓冲区不满 (使用 predicate 防止虚假唤醒)
        // wait(lock, predicate) 等价于 while(!predicate()) { wait(lock); }
        if (!m_cv_not_full.wait_for(lock, std::chrono::milliseconds(100), [this] { 
            return m_buffer.size() < MAX_BUFFER_SIZE || !m_running; 
        })) {
            // 超时或被唤醒但条件不满足（继续循环）
            continue; 
        }
        
        if (!m_running) break;
        
        // 生产数据
        int data = ++m_producedCount;
        m_buffer.push(data);
        
        logMessage(QString("生产者 生产了数据: %1 (缓冲: %2)").arg(data).arg(m_buffer.size()));
        
        // 通知消费者
        m_cv_not_empty.notify_one();
        
        lock.unlock(); // 手动解锁，让出互斥量
        
        // 模拟耗时
        std::this_thread::sleep_for(std::chrono::milliseconds(m_producerSpeed->value()));
    }
}

void ConditionVariableWidget::consumerThread(int id)
{
    while (m_running) {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        // 等待缓冲区不空
        if (!m_cv_not_empty.wait_for(lock, std::chrono::milliseconds(100), [this] {
            return !m_buffer.empty() || !m_running;
        })) {
            continue;
        }
        
        if (!m_running) break;
        
        // 消费数据
        int data = m_buffer.front();
        m_buffer.pop();
        m_consumedCount++;
        
        logMessage(QString("消费者#%1 消费了数据: %2 (缓冲: %3)").arg(id).arg(data).arg(m_buffer.size()));
        
        // 通知生产者（缓冲区有了空位）
        m_cv_not_full.notify_one();
        
        lock.unlock();
        
        // 模拟耗时
        std::this_thread::sleep_for(std::chrono::milliseconds(m_consumerSpeed->value()));
    }
}

void ConditionVariableWidget::logMessage(const QString &msg)
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    m_pendingLogs += QString("[%1] %2\n").arg(timeStr, msg);
}

void ConditionVariableWidget::updateUI()
{
    // 更新日志
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        if (!m_pendingLogs.isEmpty()) {
            m_logDisplay->append(m_pendingLogs.trimmed());
            m_pendingLogs.clear();
        }
    }
    
    // 更新缓冲区状态（不需要锁，因为这只是为了UI显示，稍微不一致没关系，且queue::size()通常是原子的或安全的只读）
    // 为了严谨，还是加个锁
    size_t size = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size = m_buffer.size();
    }
    m_bufferBar->setValue(static_cast<int>(size));
    
    if (m_running) {
        m_statusLabel->setText(QString("运行中 - 生产总数: %1, 消费总数: %2")
                               .arg(m_producedCount).arg(m_consumedCount));
    }
}

void ConditionVariableWidget::clearLog()
{
    m_logDisplay->clear();
}
