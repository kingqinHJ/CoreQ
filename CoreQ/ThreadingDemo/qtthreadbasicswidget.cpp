#include "qtthreadbasicswidget.h"
#include <QDateTime>
#include <QDebug>

// ==========================================
// Worker 实现
// ==========================================
void Worker::doWork(const QString &parameter)
{
    QString result;
    // 模拟耗时操作 (5秒)
    for (int i = 1; i <= 5; ++i) {
        // 检查线程是否请求中断 (仅在Qt5.2+ QThread::requestInterruption配合下有效，
        // 但Worker模式通常通过标志位或析构控制，这里简单模拟)
        QThread::msleep(500); 
        result = QString("步骤 %1 完成").arg(i);
        emit resultReady(QString("%1 -> %2").arg(parameter).arg(result));
    }
    emit resultReady("Worker: 所有任务完成");
    emit workFinished();
}

// ==========================================
// MyThread 实现
// ==========================================
void MyThread::run()
{
    // run() 函数内的代码在子线程执行
    for (int i = 1; i <= 3; ++i) {
        if (isInterruptionRequested()) return;
        QThread::msleep(800);
        emit resultReady(QString("SubclassThread: 阶段 %1 完成 (Thread ID: %2)")
                         .arg(i)
                         .arg(reinterpret_cast<quintptr>(QThread::currentThreadId())));
    }
    emit resultReady("SubclassThread: 执行完毕");
}

// ==========================================
// QtThreadBasicsWidget 实现
// ==========================================
QtThreadBasicsWidget::QtThreadBasicsWidget(QWidget *parent)
    : QWidget(parent)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
    , m_subclassThread(nullptr)
{
    // 布局设置
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QGroupBox *grpWorker = new QGroupBox("1. Worker-Object 模式 (推荐)", this);
    QHBoxLayout *workerLayout = new QHBoxLayout(grpWorker);
    m_btnStartWorker = new QPushButton("启动 Worker 线程", this);
    m_btnStopWorker = new QPushButton("安全停止 Worker", this);
    m_btnStopWorker->setEnabled(false);
    workerLayout->addWidget(m_btnStartWorker);
    workerLayout->addWidget(m_btnStopWorker);

    QGroupBox *grpSubclass = new QGroupBox("2. QThread 子类化模式 (传统)", this);
    QHBoxLayout *subclassLayout = new QHBoxLayout(grpSubclass);
    m_btnStartSubclass = new QPushButton("启动子类化线程", this);
    subclassLayout->addWidget(m_btnStartSubclass);

    m_logDisplay = new QTextEdit(this);
    m_logDisplay->setReadOnly(true);

    QPushButton *btnClear = new QPushButton("清空日志", this);
    connect(btnClear, &QPushButton::clicked, this, &QtThreadBasicsWidget::clearLog);

    mainLayout->addWidget(grpWorker);
    mainLayout->addWidget(grpSubclass);
    mainLayout->addWidget(new QLabel("运行日志:", this));
    mainLayout->addWidget(m_logDisplay);
    mainLayout->addWidget(btnClear);

    // 信号槽连接
    connect(m_btnStartWorker, &QPushButton::clicked, this, &QtThreadBasicsWidget::startWorker);
    connect(m_btnStopWorker, &QPushButton::clicked, this, &QtThreadBasicsWidget::stopWorker);
    connect(m_btnStartSubclass, &QPushButton::clicked, this, &QtThreadBasicsWidget::startSubclassThread);
}

QtThreadBasicsWidget::~QtThreadBasicsWidget()
{
    // 析构时确保线程安全退出
    stopWorker();
    
    if (m_subclassThread) {
        m_subclassThread->requestInterruption();
        m_subclassThread->quit();
        m_subclassThread->wait();
        delete m_subclassThread;
    }
}

void QtThreadBasicsWidget::startWorker()
{
    if (m_workerThread) return;

    logMessage("准备启动 Worker 模式...");

    // 1. 创建线程对象和Worker对象
    m_workerThread = new QThread;
    m_worker = new Worker; // 不能指定parent，否则无法moveToThread

    // 2. 将Worker移入线程
    m_worker->moveToThread(m_workerThread);

    // 3. 连接信号槽
    // 这里的连接非常关键：
    // - thread.started -> worker.doWork: 线程启动时自动开始工作
    // - worker.resultReady -> this.handle: 获取结果
    // - worker.workFinished -> thread.quit: 工作做完让线程退出事件循环
    // - thread.finished -> thread.deleteLater: 线程结束后自动删除QThread对象
    // - thread.finished -> worker.deleteLater: 线程结束后自动删除Worker对象
    
    connect(m_workerThread, &QThread::started, m_worker, [this](){
        m_worker->doWork("任务A");
    });
    
    connect(m_worker, &Worker::resultReady, this, &QtThreadBasicsWidget::handleWorkerResult);
    
    // 这里的连接决定了何时结束线程
    // 方案A：任务做完就结束线程
    connect(m_worker, &Worker::workFinished, this, &QtThreadBasicsWidget::handleWorkerFinished);
    
    // 资源自动清理
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
    
    // 4. 启动线程
    m_workerThread->start();
    
    m_btnStartWorker->setEnabled(false);
    m_btnStopWorker->setEnabled(true);
    logMessage(QString("Worker 线程已启动 (Thread ID: %1)").arg(reinterpret_cast<quintptr>(m_workerThread)));
}

void QtThreadBasicsWidget::stopWorker()
{
    if (!m_workerThread) return;

    logMessage("正在停止 Worker 线程...");
    
    // 请求退出
    m_workerThread->quit();
    // 阻塞等待退出
    m_workerThread->wait();
    
    // 指针置空（对象会在deleteLater中删除，但指针本身变成悬空指针，所以要置空）
    // 注意：由于deleteLater是异步的，严格来说这里置空后，原对象可能还没析构。
    // 但在demo中我们通过wait()保证了线程已停止。
    m_workerThread = nullptr;
    m_worker = nullptr;

    m_btnStartWorker->setEnabled(true);
    m_btnStopWorker->setEnabled(false);
    logMessage("Worker 线程已安全停止");
}

void QtThreadBasicsWidget::handleWorkerResult(const QString &result)
{
    logMessage(QString("[Worker回调] %1").arg(result));
}

void QtThreadBasicsWidget::handleWorkerFinished()
{
    logMessage("Worker 报告任务完成");
    // 可以在这里触发 stopWorker() 或者让它自动清理
    // 在本例中，stopWorker会被调用以重置UI状态
    stopWorker();
}

void QtThreadBasicsWidget::startSubclassThread()
{
    if (m_subclassThread && m_subclassThread->isRunning()) {
        logMessage("子类化线程正在运行中...");
        return;
    }

    // 清理旧对象
    if (m_subclassThread) {
        m_subclassThread->wait();
        delete m_subclassThread;
    }

    m_subclassThread = new MyThread(this);
    connect(m_subclassThread, &MyThread::resultReady, this, &QtThreadBasicsWidget::handleSubclassResult);
    connect(m_subclassThread, &QThread::finished, [this](){
        logMessage("子类化线程已结束");
        m_btnStartSubclass->setEnabled(true);
    });

    m_subclassThread->start();
    m_btnStartSubclass->setEnabled(false);
    logMessage("子类化线程已启动");
}

void QtThreadBasicsWidget::handleSubclassResult(const QString &result)
{
    logMessage(QString("[Subclass回调] %1").arg(result));
}

void QtThreadBasicsWidget::clearLog()
{
    m_logDisplay->clear();
}

void QtThreadBasicsWidget::logMessage(const QString &msg)
{
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    m_logDisplay->append(QString("[%1] %2").arg(timeStr, msg));
}
