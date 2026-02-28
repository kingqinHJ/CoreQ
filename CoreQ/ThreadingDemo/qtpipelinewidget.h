#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QAtomicInt>

// ============================================================================
// 知识点：流水线（Pipeline）设计模式
// ============================================================================
// 流水线模式是一种并行设计模式，将任务分解为多个顺序执行的阶段（Stage），
// 每个阶段由独立线程处理，阶段之间通过线程安全队列传递数据。
//
// 核心优势：
// 1. 提高吞吐量：多个任务可以同时在不同阶段并行处理
// 2. 解耦合：各阶段独立，可独立优化和扩展
// 3. 负载均衡：各阶段可以有不同的处理速度
//
// 典型应用场景：
// - 音视频处理（采集->编码->传输）
// - 编译器（词法分析->语法分析->语义分析->代码生成）
// - 数据处理流水线（读取->清洗->转换->存储）
// - 图像处理（加载->滤镜->保存）

// ============================================================================
// 知识点：线程安全队列（Thread-Safe Queue）
// ============================================================================
// 线程安全队列是流水线模式的核心组件，用于在不同阶段之间传递数据。
// 需要满足：
// 1. 线程安全：多个线程同时访问时数据不会损坏
// 2. 阻塞/非阻塞：支持在队列为空时阻塞消费者，或超时返回
// 3. 优雅退出：支持安全地停止队列，避免线程死锁

template<typename T>
class PipelineQueue
{
public:
    // 入队操作
    void enqueue(const T &data)
    {
        QMutexLocker locker(&m_mutex);
        m_queue.enqueue(data);
        m_condition.wakeOne();  // 唤醒等待的消费者线程
    }

    // 出队操作（阻塞式）
    bool dequeue(T &data, int timeoutMs = 100)
    {
        QMutexLocker locker(&m_mutex);
        // wait() 会自动解锁 mutex，等待被唤醒后重新加锁
        if (!m_condition.wait(&m_mutex, timeoutMs)) {
            return false;  // 超时
        }
        if (m_queue.isEmpty()) {
            return false;
        }
        data = m_queue.dequeue();
        return true;
    }

    // 非阻塞式出队
    bool tryDequeue(T &data)
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.isEmpty()) {
            return false;
        }
        data = m_queue.dequeue();
        return true;
    }

    // 清空队列
    void clear()
    {
        QMutexLocker locker(&m_mutex);
        m_queue.clear();
    }

    // 唤醒所有等待的线程（用于停止流水线）
    void wakeAll()
    {
        m_condition.wakeAll();
    }

    bool isEmpty()
    {
        QMutexLocker locker(&m_mutex);
        return m_queue.isEmpty();
    }

    int size()
    {
        QMutexLocker locker(&m_mutex);
        return m_queue.size();
    }

private:
    QMutex m_mutex;              // 保护队列的互斥锁
    QWaitCondition m_condition;  // 条件变量，用于线程同步
    QQueue<T> m_queue;           // 底层队列
};

// ============================================================================
// 知识点：流水线数据包
// ============================================================================
// 数据包是在流水线各阶段之间传递的数据单元。
// 可以包含原始数据、处理状态、元数据等。
struct PipelineData
{
    int taskId;           // 任务ID，用于追踪
    QString rawData;      // 原始数据（Stage1采集的数据）
    QString processedData; // 处理后数据（Stage2处理的结果）
    qint64 timestamp;     // 时间戳，用于计算处理延迟

    PipelineData() : taskId(0), timestamp(0) {}
};

// 前向声明
class Stage1Worker;
class Stage2Worker;
class Stage3Worker;

class QtPipelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtPipelineWidget(QWidget *parent = nullptr);
    ~QtPipelineWidget();

private slots:
    void onStartClicked();
    void onStopClicked();

    // 接收各阶段的状态更新信号
    void onStage1Status(const QString &status, int taskId);
    void onStage2Status(const QString &status, int taskId);
    void onStage3Status(const QString &status, int taskId);
    void onTaskCompleted(int taskId);

private:
    void setupUi();
    void logMessage(const QString &msg);
    void updateStageLabel(QLabel *label, const QString &text, bool active);

    QPushButton *m_btnStart;
    QPushButton *m_btnStop;

    // 模拟三个阶段的状态显示
    QLabel *m_lblStage1; // 采集
    QLabel *m_lblStage2; // 处理
    QLabel *m_lblStage3; // 显示/存储

    QLabel *m_lblCount; // 总处理数
    QTextEdit *m_logViewer;

    // ============================================================================
    // 知识点：流水线各阶段的工作线程
    // ============================================================================
    // 每个阶段运行在独立的线程中，通过线程安全队列传递数据。
    // 这种设计实现了真正的并行处理：当Stage2处理第N个数据时，
    // Stage1可以同时采集第N+1个数据。

    // Stage1 -> Stage2 的队列
    PipelineQueue<PipelineData> m_queue1to2;
    // Stage2 -> Stage3 的队列
    PipelineQueue<PipelineData> m_queue2to3;

    // 各阶段的工作线程
    QThread *m_threadStage1;
    QThread *m_threadStage2;
    QThread *m_threadStage3;

    // 各阶段的工作对象
    Stage1Worker *m_workerStage1;
    Stage2Worker *m_workerStage2;
    Stage3Worker *m_workerStage3;

    // 运行状态标志
    QAtomicInt m_running;  // 原子操作，线程安全的状态标志
    int m_completedCount;  // 已完成任务计数
};

// ============================================================================
// 知识点：QObject 线程亲和性（Thread Affinity）
// ============================================================================
// QObject 有"线程亲和性"概念，即 QObject 属于创建它的线程。
// 使用 moveToThread() 可以将对象移动到指定线程执行。
// 信号槽机制会自动处理跨线程通信，使用队列连接（Queued Connection）。

// Stage 1: 数据采集阶段
// 职责：模拟从外部源采集原始数据
class Stage1Worker : public QObject
{
    Q_OBJECT
public:
    explicit Stage1Worker(PipelineQueue<PipelineData> *outputQueue,
                          QAtomicInt *running,
                          QObject *parent = nullptr);

public slots:
    void startWork();  // 开始工作

signals:
    void statusChanged(const QString &status, int taskId);  // 状态更新信号
    void logMessage(const QString &msg);                     // 日志信号

private:
    PipelineQueue<PipelineData> *m_outputQueue;
    QAtomicInt *m_running;
    int m_taskIdCounter;
};

// Stage 2: 数据处理阶段
// 职责：对采集的数据进行处理/转换
class Stage2Worker : public QObject
{
    Q_OBJECT
public:
    explicit Stage2Worker(PipelineQueue<PipelineData> *inputQueue,
                          PipelineQueue<PipelineData> *outputQueue,
                          QAtomicInt *running,
                          QObject *parent = nullptr);

public slots:
    void startWork();

signals:
    void statusChanged(const QString &status, int taskId);
    void logMessage(const QString &msg);

private:
    PipelineQueue<PipelineData> *m_inputQueue;
    PipelineQueue<PipelineData> *m_outputQueue;
    QAtomicInt *m_running;
};

// Stage 3: 数据存储/显示阶段
// 职责：将处理后的数据存储或显示
class Stage3Worker : public QObject
{
    Q_OBJECT
public:
    explicit Stage3Worker(PipelineQueue<PipelineData> *inputQueue,
                          QAtomicInt *running,
                          QObject *parent = nullptr);

public slots:
    void startWork();

signals:
    void statusChanged(const QString &status, int taskId);
    void taskCompleted(int taskId);  // 任务完成信号
    void logMessage(const QString &msg);

private:
    PipelineQueue<PipelineData> *m_inputQueue;
    QAtomicInt *m_running;
};
