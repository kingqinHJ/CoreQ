#include "qtpipelinewidget.h"
#include <QDateTime>
#include <QThread>
#include <QRandomGenerator>

// ============================================================================
// QtPipelineWidget 实现
// ============================================================================

QtPipelineWidget::QtPipelineWidget(QWidget *parent)
    : QWidget(parent)
    , m_threadStage1(nullptr)
    , m_threadStage2(nullptr)
    , m_threadStage3(nullptr)
    , m_workerStage1(nullptr)
    , m_workerStage2(nullptr)
    , m_workerStage3(nullptr)
    , m_running(0)
    , m_completedCount(0)
{
    setupUi();
}

QtPipelineWidget::~QtPipelineWidget()
{
    // 确保停止流水线
    onStopClicked();
}

void QtPipelineWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnStart = new QPushButton("启动流水线", this);
    m_btnStop = new QPushButton("停止流水线", this);
    m_btnStop->setEnabled(false);
    btnLayout->addWidget(m_btnStart);
    btnLayout->addWidget(m_btnStop);
    mainLayout->addLayout(btnLayout);

    // 流水线可视化
    QGroupBox *grpPipeline = new QGroupBox("流水线状态 (Stage 1 -> Stage 2 -> Stage 3)", this);
    QHBoxLayout *pipeLayout = new QHBoxLayout(grpPipeline);

    auto createStageLabel = [this](const QString &text) {
        QLabel *lbl = new QLabel(text, this);
        lbl->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setFixedSize(120, 80);
        lbl->setStyleSheet("background-color: lightgray; font-size: 12px;");
        return lbl;
    };

    m_lblStage1 = createStageLabel("Stage 1\n(数据采集)");
    m_lblStage2 = createStageLabel("Stage 2\n(数据处理)");
    m_lblStage3 = createStageLabel("Stage 3\n(数据存储)");

    pipeLayout->addStretch();
    pipeLayout->addWidget(m_lblStage1);
    pipeLayout->addWidget(new QLabel("-->"));
    pipeLayout->addWidget(m_lblStage2);
    pipeLayout->addWidget(new QLabel("-->"));
    pipeLayout->addWidget(m_lblStage3);
    pipeLayout->addStretch();

    mainLayout->addWidget(grpPipeline);

    // 统计
    QHBoxLayout *statLayout = new QHBoxLayout();
    statLayout->addWidget(new QLabel("已完成任务数:"));
    m_lblCount = new QLabel("0", this);
    statLayout->addWidget(m_lblCount);
    statLayout->addStretch();
    mainLayout->addLayout(statLayout);

    m_logViewer = new QTextEdit(this);
    m_logViewer->setReadOnly(true);
    mainLayout->addWidget(m_logViewer);

    connect(m_btnStart, &QPushButton::clicked, this, &QtPipelineWidget::onStartClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &QtPipelineWidget::onStopClicked);
}

void QtPipelineWidget::updateStageLabel(QLabel *label, const QString &text, bool active)
{
    // 在主线程中更新UI
    QString color = active ? "lightgreen" : "lightgray";
    label->setStyleSheet(QString("background-color: %1; font-size: 12px;").arg(color));
    label->setText(text);
}

void QtPipelineWidget::onStartClicked()
{
    logMessage("启动流水线...");
    m_btnStart->setEnabled(false);
    m_btnStop->setEnabled(true);
    m_completedCount = 0;
    m_lblCount->setText("0");

    // ============================================================================
    // 知识点：流水线启动流程
    // ============================================================================
    // 1. 设置运行标志
    // 2. 创建并启动各阶段线程
    // 3. 连接信号槽用于状态更新
    // 4. 触发各阶段开始工作

    m_running.storeRelease(1);

    // 创建 Stage 1 线程（数据采集）
    m_threadStage1 = new QThread(this);
    m_workerStage1 = new Stage1Worker(&m_queue1to2, &m_running);
    m_workerStage1->moveToThread(m_threadStage1);

    connect(m_threadStage1, &QThread::started, m_workerStage1, &Stage1Worker::startWork);
    connect(m_workerStage1, &Stage1Worker::statusChanged, this, &QtPipelineWidget::onStage1Status);
    connect(m_workerStage1, &Stage1Worker::logMessage, this, &QtPipelineWidget::logMessage);
    connect(m_threadStage1, &QThread::finished, m_workerStage1, &QObject::deleteLater);

    // 创建 Stage 2 线程（数据处理）
    m_threadStage2 = new QThread(this);
    m_workerStage2 = new Stage2Worker(&m_queue1to2, &m_queue2to3, &m_running);
    m_workerStage2->moveToThread(m_threadStage2);

    connect(m_threadStage2, &QThread::started, m_workerStage2, &Stage2Worker::startWork);
    connect(m_workerStage2, &Stage2Worker::statusChanged, this, &QtPipelineWidget::onStage2Status);
    connect(m_workerStage2, &Stage2Worker::logMessage, this, &QtPipelineWidget::logMessage);
    connect(m_threadStage2, &QThread::finished, m_workerStage2, &QObject::deleteLater);

    // 创建 Stage 3 线程（数据存储）
    m_threadStage3 = new QThread(this);
    m_workerStage3 = new Stage3Worker(&m_queue2to3, &m_running);
    m_workerStage3->moveToThread(m_threadStage3);

    connect(m_threadStage3, &QThread::started, m_workerStage3, &Stage3Worker::startWork);
    connect(m_workerStage3, &Stage3Worker::statusChanged, this, &QtPipelineWidget::onStage3Status);
    connect(m_workerStage3, &Stage3Worker::logMessage, this, &QtPipelineWidget::logMessage);
    connect(m_workerStage3, &Stage3Worker::taskCompleted, this, &QtPipelineWidget::onTaskCompleted);
    connect(m_threadStage3, &QThread::finished, m_workerStage3, &QObject::deleteLater);

    // 启动所有线程
    m_threadStage3->start();
    m_threadStage2->start();
    m_threadStage1->start();

    logMessage("流水线已启动，三个工作线程正在运行...");
}

void QtPipelineWidget::onStopClicked()
{
    logMessage("正在停止流水线...");

    // ============================================================================
    // 知识点：流水线优雅停止
    // ============================================================================
    // 1. 设置停止标志，通知各阶段停止生产新数据
    // 2. 唤醒所有等待的线程，让它们检查停止标志
    // 3. 等待各线程处理完当前数据并退出
    // 4. 清理资源

    // 设置停止标志
    m_running.storeRelease(0);

    // 唤醒所有等待的线程，让它们检查停止标志
    m_queue1to2.wakeAll();
    m_queue2to3.wakeAll();

    // 等待线程结束
    if (m_threadStage1 && m_threadStage1->isRunning()) {
        m_threadStage1->quit();
        m_threadStage1->wait(2000);
    }
    if (m_threadStage2 && m_threadStage2->isRunning()) {
        m_threadStage2->quit();
        m_threadStage2->wait(2000);
    }
    if (m_threadStage3 && m_threadStage3->isRunning()) {
        m_threadStage3->quit();
        m_threadStage3->wait(2000);
    }

    // 清理线程对象
    delete m_threadStage1;
    delete m_threadStage2;
    delete m_threadStage3;
    m_threadStage1 = nullptr;
    m_threadStage2 = nullptr;
    m_threadStage3 = nullptr;
    m_workerStage1 = nullptr;
    m_workerStage2 = nullptr;
    m_workerStage3 = nullptr;

    // 清空队列
    m_queue1to2.clear();
    m_queue2to3.clear();

    // 重置UI状态
    m_btnStart->setEnabled(true);
    m_btnStop->setEnabled(false);
    updateStageLabel(m_lblStage1, "Stage 1\n(数据采集)", false);
    updateStageLabel(m_lblStage2, "Stage 2\n(数据处理)", false);
    updateStageLabel(m_lblStage3, "Stage 3\n(数据存储)", false);

    logMessage("流水线已停止");
}

void QtPipelineWidget::onStage1Status(const QString &status, int taskId)
{
    Q_UNUSED(taskId)
    updateStageLabel(m_lblStage1, QString("Stage 1\n(数据采集)\n%1").arg(status), true);
}

void QtPipelineWidget::onStage2Status(const QString &status, int taskId)
{
    Q_UNUSED(taskId)
    updateStageLabel(m_lblStage2, QString("Stage 2\n(数据处理)\n%1").arg(status), true);
}

void QtPipelineWidget::onStage3Status(const QString &status, int taskId)
{
    Q_UNUSED(taskId)
    updateStageLabel(m_lblStage3, QString("Stage 3\n(数据存储)\n%1").arg(status), true);
}

void QtPipelineWidget::onTaskCompleted(int taskId)
{
    m_completedCount++;
    m_lblCount->setText(QString::number(m_completedCount));

    // 每完成10个任务输出一次统计
    if (m_completedCount % 10 == 0) {
        logMessage(QString("已完成 %1 个任务").arg(m_completedCount));
    }

    // 重置Stage1和Stage2的显示（表示它们当前空闲）
    Q_UNUSED(taskId)
}

void QtPipelineWidget::logMessage(const QString &msg)
{
    m_logViewer->append(QString("[%1] %2")
                        .arg(QDateTime::currentDateTime().toString("HH:mm:ss"), msg));
}

// ============================================================================
// Stage1Worker 实现：数据采集阶段
// ============================================================================
// 知识点：生产者模式
// Stage1是流水线的起点，负责生成数据并放入队列。
// 它只生产数据，不消费数据。

Stage1Worker::Stage1Worker(PipelineQueue<PipelineData> *outputQueue,
                           QAtomicInt *running,
                           QObject *parent)
    : QObject(parent)
    , m_outputQueue(outputQueue)
    , m_running(running)
    , m_taskIdCounter(0)
{
}

void Stage1Worker::startWork()
{
    logMessage("[Stage1] 数据采集线程已启动");

    while (m_running->loadAcquire()) {
        // 模拟数据采集（如从传感器、文件、网络读取数据）
        emit statusChanged("采集中...", m_taskIdCounter);

        // 模拟采集耗时（100-300ms）
        QThread::msleep(100 + QRandomGenerator::global()->bounded(200));

        // 创建数据包
        PipelineData data;
        data.taskId = ++m_taskIdCounter;
        data.rawData = QString("RawData_%1").arg(data.taskId);
        data.timestamp = QDateTime::currentMSecsSinceEpoch();

        // 将数据放入队列，传递给Stage2
        m_outputQueue->enqueue(data);

        emit statusChanged(QString("已发送 #%1").arg(data.taskId), data.taskId);
        emit logMessage(QString("[Stage1] 采集数据 #%1: %2")
                        .arg(data.taskId).arg(data.rawData));
    }

    emit logMessage("[Stage1] 数据采集线程已停止");
}

// ============================================================================
// Stage2Worker 实现：数据处理阶段
// ============================================================================
// 知识点：消费者+生产者模式（中间阶段）
// Stage2既是消费者（从Stage1接收数据），又是生产者（向Stage3发送数据）。
// 这种设计允许Stage2对数据进行转换、过滤、增强等操作。

Stage2Worker::Stage2Worker(PipelineQueue<PipelineData> *inputQueue,
                           PipelineQueue<PipelineData> *outputQueue,
                           QAtomicInt *running,
                           QObject *parent)
    : QObject(parent)
    , m_inputQueue(inputQueue)
    , m_outputQueue(outputQueue)
    , m_running(running)
{
}

void Stage2Worker::startWork()
{
    logMessage("[Stage2] 数据处理线程已启动");

    PipelineData data;
    while (m_running->loadAcquire()) {
        // 从输入队列获取数据（阻塞等待）
        if (!m_inputQueue->dequeue(data, 100)) {
            // 超时，继续检查运行状态
            continue;
        }

        emit statusChanged(QString("处理 #%1...").arg(data.taskId), data.taskId);

        // 模拟数据处理（如解析、计算、格式转换）
        // 处理时间比采集时间长，模拟计算密集型任务
        QThread::msleep(200 + QRandomGenerator::global()->bounded(300));

        // 处理数据：将原始数据转换为大写，表示"处理"
        data.processedData = data.rawData.toUpper();

        emit statusChanged(QString("完成 #%1").arg(data.taskId), data.taskId);
        emit logMessage(QString("[Stage2] 处理数据 #%1: %2 -> %3")
                        .arg(data.taskId)
                        .arg(data.rawData)
                        .arg(data.processedData));

        // 将处理后的数据放入输出队列，传递给Stage3
        m_outputQueue->enqueue(data);
    }

    emit logMessage("[Stage2] 数据处理线程已停止");
}

// ============================================================================
// Stage3Worker 实现：数据存储阶段
// ============================================================================
// 知识点：消费者模式
// Stage3是流水线的终点，负责消费数据（存储到数据库、写入文件、显示等）。
// 它只消费数据，不生产数据。

Stage3Worker::Stage3Worker(PipelineQueue<PipelineData> *inputQueue,
                           QAtomicInt *running,
                           QObject *parent)
    : QObject(parent)
    , m_inputQueue(inputQueue)
    , m_running(running)
{
}

void Stage3Worker::startWork()
{
    logMessage("[Stage3] 数据存储线程已启动");

    PipelineData data;
    while (m_running->loadAcquire()) {
        // 从输入队列获取数据（阻塞等待）
        if (!m_inputQueue->dequeue(data, 100)) {
            // 超时，继续检查运行状态
            continue;
        }

        emit statusChanged(QString("存储 #%1...").arg(data.taskId), data.taskId);

        // 模拟数据存储（如写入数据库、文件、网络发送）
        QThread::msleep(150 + QRandomGenerator::global()->bounded(100));

        // 计算处理延迟
        qint64 latency = QDateTime::currentMSecsSinceEpoch() - data.timestamp;

        emit statusChanged(QString("已存储 #%1").arg(data.taskId), data.taskId);
        emit logMessage(QString("[Stage3] 存储数据 #%1: %2 (延迟: %3ms)")
                        .arg(data.taskId)
                        .arg(data.processedData)
                        .arg(latency));

        // 通知任务完成
        emit taskCompleted(data.taskId);
    }

    emit logMessage("[Stage3] 数据存储线程已停止");
}
