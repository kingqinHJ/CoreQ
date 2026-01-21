#include "qtconcurrentwidget.h"
#include <QDateTime>
#include <QThread>

// 辅助耗时函数：无返回值
void simpleFunction(int seconds) {
    QThread::sleep(seconds);
}

// 辅助耗时函数：带返回值
int calculateFunction(int base) {
    QThread::sleep(2);
    return base * base;
}

// 辅助耗时函数：支持进度的任务 (改为 mapped 版本以演示返回值和进度)
int mappedFunction(const int &item) {
    QThread::msleep(100); // 模拟每个项处理100ms
    return item * 2;
}

QtConcurrentWidget::QtConcurrentWidget(QWidget *parent)
    : QWidget(parent)
    , m_voidWatcher(new QFutureWatcher<void>(this))
    , m_intWatcher(new QFutureWatcher<int>(this))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 控制区
    QGroupBox *grpControls = new QGroupBox("任务控制", this);
    QHBoxLayout *btnLayout = new QHBoxLayout(grpControls);
    
    m_btnSimpleRun = new QPushButton("1. 简单异步任务 (Run)", this);
    m_btnResultRun = new QPushButton("2. 带返回值的任务", this);
    m_btnProgressRun = new QPushButton("3. 带进度的批处理 (Map)", this);
    m_btnCancel = new QPushButton("取消任务", this);
    m_btnCancel->setEnabled(false);

    btnLayout->addWidget(m_btnSimpleRun);
    btnLayout->addWidget(m_btnResultRun);
    btnLayout->addWidget(m_btnProgressRun);
    btnLayout->addWidget(m_btnCancel);

    // 状态显示
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    
    m_statusLabel = new QLabel("就绪", this);
    
    m_logDisplay = new QTextEdit(this);
    m_logDisplay->setReadOnly(true);
    
    QPushButton *btnClear = new QPushButton("清空日志", this);

    mainLayout->addWidget(grpControls);
    mainLayout->addWidget(new QLabel("任务进度:", this));
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(new QLabel("运行日志:", this));
    mainLayout->addWidget(m_logDisplay);
    mainLayout->addWidget(btnClear);

    // 连接信号槽
    connect(m_btnSimpleRun, &QPushButton::clicked, this, &QtConcurrentWidget::runSimpleTask);
    connect(m_btnResultRun, &QPushButton::clicked, this, &QtConcurrentWidget::runTaskWithResult);
    connect(m_btnProgressRun, &QPushButton::clicked, this, &QtConcurrentWidget::runTaskWithProgress);
    connect(m_btnCancel, &QPushButton::clicked, this, &QtConcurrentWidget::cancelTask);
    connect(btnClear, &QPushButton::clicked, this, &QtConcurrentWidget::clearLog);

    // 连接 Watcher 信号
    connect(m_voidWatcher, &QFutureWatcher<void>::finished, this, &QtConcurrentWidget::onTaskFinished);
    connect(m_voidWatcher, &QFutureWatcher<void>::canceled, this, &QtConcurrentWidget::onTaskCanceled);
    
    connect(m_intWatcher, &QFutureWatcher<int>::finished, this, &QtConcurrentWidget::onTaskFinished);
    // 注意：resultReadyAt 是针对 map/filter 等返回多个结果的，
    // 对于 run 返回单个结果，通常在 finished 后直接用 future().result() 获取
    
    // 进度信号 (主要针对 map/filter)
    connect(m_intWatcher, &QFutureWatcher<int>::progressValueChanged, this, &QtConcurrentWidget::onTaskProgressValueChanged);
    connect(m_intWatcher, &QFutureWatcher<int>::progressRangeChanged, m_progressBar, &QProgressBar::setRange);
}

QtConcurrentWidget::~QtConcurrentWidget()
{
    // 退出前取消正在运行的任务
    if (m_voidWatcher->isRunning()) {
        m_voidWatcher->cancel();
        m_voidWatcher->waitForFinished();
    }
    if (m_intWatcher->isRunning()) {
        m_intWatcher->cancel();
        m_intWatcher->waitForFinished();
    }
}

void QtConcurrentWidget::runSimpleTask()
{
    logMessage("启动简单任务：睡眠 2 秒...");
    m_statusLabel->setText("正在运行简单任务...");
    m_btnSimpleRun->setEnabled(false);
    
    // QtConcurrent::run 会在全局线程池中找一个线程运行函数
    QFuture<void> future = QtConcurrent::run(simpleFunction, 2);
    m_voidWatcher->setFuture(future);
}

void QtConcurrentWidget::runTaskWithResult()
{
    logMessage("启动计算任务：计算 10 的平方...");
    m_statusLabel->setText("正在运行计算任务...");
    m_btnResultRun->setEnabled(false);
    
    // 使用 lambda 表达式
    QFuture<int> future = QtConcurrent::run([](){
        QThread::sleep(2);
        return 10 * 10;
    });

    // ---------------------------------------------------------
    // 同步写法示例 (会卡住界面)
    // ---------------------------------------------------------
    // future.waitForFinished(); // 等待任务结束，主线程会卡死在这里 2 秒
    // int result = future.result(); // 获取结果，如果任务没结束也会阻塞等待
    // logMessage(QString("同步获取结果: %1").arg(result));
    // ---------------------------------------------------------

    // ---------------------------------------------------------
    // 异步写法 (推荐)
    // ---------------------------------------------------------
    // 将 Future 交给 Watcher 监控，通过信号槽获取结果，不会阻塞主线程
    m_intWatcher->setFuture(future);
}

void QtConcurrentWidget::runTaskWithProgress()
{
    logMessage("启动批处理任务：处理 100 个数据项...");
    m_statusLabel->setText("正在运行批处理...");
    m_btnProgressRun->setEnabled(false);
    m_btnCancel->setEnabled(true);
    
    // 准备数据
    static QList<int> list;
    list.clear();
    for (int i = 0; i < 100; ++i) list.append(i);
    
    // 使用 mapped 函数并行处理列表中的每一项
    // mapped 会根据 CPU 核心数自动分配线程
    // QFuture 能够报告处理进度，并返回处理后的新列表
    QFuture<int> future = QtConcurrent::mapped(list, mappedFunction);
    
    m_intWatcher->setFuture(future);
}

void QtConcurrentWidget::onTaskFinished()
{
    // 判断是哪个 Watcher 触发的
    if (sender() == m_voidWatcher) {
        logMessage("简单任务完成");
        m_btnSimpleRun->setEnabled(true);
    } 
    else if (sender() == m_intWatcher) {
        // 如果是批处理任务 (结果数量大于1，或者是取消状态且有部分结果)
        // 简单判断：如果之前是批处理按钮禁用了，说明是批处理
        if (!m_btnProgressRun->isEnabled()) {
             logMessage(QString("批处理任务完成，共处理 %1 项").arg(m_intWatcher->future().resultCount()));
             m_btnProgressRun->setEnabled(true);
        } else {
            // 是单个计算任务
            logMessage(QString("计算任务完成，结果: %1").arg(m_intWatcher->result()));
            m_btnResultRun->setEnabled(true);
        }
    }
    
    m_statusLabel->setText("就绪");
    m_btnCancel->setEnabled(false);
    m_progressBar->setValue(0);
}

void QtConcurrentWidget::onTaskResultReadyAt(int index)
{
    // 仅用于 map/filter 产生结果时
    Q_UNUSED(index);
}

void QtConcurrentWidget::onTaskProgressValueChanged(int value)
{
    m_progressBar->setValue(value);
    // 可选：更新日志 (太频繁会卡顿，所以只在特定节点更新)
    if (value % 20 == 0) {
        logMessage(QString("进度: %1%").arg(value));
    }
}

void QtConcurrentWidget::onTaskCanceled()
{
    logMessage("任务被取消！");
    m_statusLabel->setText("已取消");
}

void QtConcurrentWidget::cancelTask()
{
    logMessage("正在请求取消...");
    m_intWatcher->cancel();
    // voidWatcher 本例中没有可取消的长任务，所以主要针对 map
}

void QtConcurrentWidget::clearLog()
{
    m_logDisplay->clear();
}

void QtConcurrentWidget::logMessage(const QString &msg)
{
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_logDisplay->append(QString("[%1] %2").arg(timeStr, msg));
}
