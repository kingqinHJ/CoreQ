#include "qtparallelmapwidget.h"
#include <QDateTime>
#include <QGroupBox>
#include <QRandomGenerator>
#include <algorithm>

QtParallelMapWidget::QtParallelMapWidget(QWidget *parent)
    : QWidget(parent)
    , m_intWatcher(new QFutureWatcher<int>(this))
    , m_stringWatcher(new QFutureWatcher<QString>(this))
    , m_isProcessing(false)
{
    setupUi();
}

QtParallelMapWidget::~QtParallelMapWidget()
{
    // 清理正在进行的任务
    if (m_intWatcher->isRunning()) {
        m_intWatcher->cancel();
        m_intWatcher->waitForFinished();
    }
    if (m_stringWatcher->isRunning()) {
        m_stringWatcher->cancel();
        m_stringWatcher->waitForFinished();
    }
}

void QtParallelMapWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    // 1. 操作栏
    QGroupBox *grpControls = new QGroupBox("操作控制", this);
    QHBoxLayout *topLayout = new QHBoxLayout(grpControls);
    
    m_btnLoadData = new QPushButton("1. 生成模拟数据", this);
    m_btnProcess = new QPushButton("2. 并行处理 (Map)", this);
    m_btnCancel = new QPushButton("取消任务", this);
    m_btnClear = new QPushButton("清空结果", this);
    
    m_btnProcess->setEnabled(false);
    m_btnCancel->setEnabled(false);

    topLayout->addWidget(m_btnLoadData);
    topLayout->addWidget(m_btnProcess);
    topLayout->addWidget(m_btnCancel);
    topLayout->addWidget(m_btnClear);
    topLayout->addStretch();
    
    mainLayout->addWidget(grpControls);

    // 2. 状态栏
    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("就绪 - 请点击生成数据", this);
    m_statusLabel->setStyleSheet("color: blue; font-weight: bold;");
    
    QLabel *progressLabel = new QLabel("处理进度:", this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(progressLabel);
    statusLayout->addWidget(m_progressBar);
    mainLayout->addLayout(statusLayout);

    // 3. 结果显示
    QGroupBox *grpResults = new QGroupBox("处理结果", this);
    QVBoxLayout *resLayout = new QVBoxLayout(grpResults);
    m_listResults = new QListWidget(this);
    m_listResults->setSelectionMode(QAbstractItemView::SingleSelection);
    resLayout->addWidget(m_listResults);
    mainLayout->addWidget(grpResults);

    // 4. 日志
    QGroupBox *grpLog = new QGroupBox("运行日志", this);
    QVBoxLayout *logLayout = new QVBoxLayout(grpLog);
    m_logViewer = new QTextEdit(this);
    m_logViewer->setMaximumHeight(150);
    m_logViewer->setReadOnly(true);
    m_logViewer->setFont(QFont("Consolas", 9));
    logLayout->addWidget(m_logViewer);
    mainLayout->addWidget(grpLog);

    // 连接信号槽
    connect(m_btnLoadData, &QPushButton::clicked, this, &QtParallelMapWidget::onLoadData);
    connect(m_btnProcess, &QPushButton::clicked, this, &QtParallelMapWidget::onProcessClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &QtParallelMapWidget::onCancelClicked);
    connect(m_btnClear, &QPushButton::clicked, this, &QtParallelMapWidget::clearResults);
    
    // =========================================================================
    // 知识点：QFutureWatcher 用法
    // =========================================================================
    // QFutureWatcher 是连接 QFuture (后台任务) 和 UI (主线程) 的桥梁。
    // 它通过信号槽机制，让我们能"异步"地获取进度和结果，而不会卡死界面。
    // 
    // 常用信号：
    // - finished(): 任务全部完成时触发
    // - canceled(): 任务被取消时触发
    // - progressValueChanged(int): 进度数值变化 (0-100)
    // - resultReadyAt(int): 某一项的结果计算好了 (用于实时显示)
    // =========================================================================

    // 连接 FutureWatcher 信号
    connect(m_intWatcher, &QFutureWatcher<int>::finished, this, &QtParallelMapWidget::onProcessingFinished);
    connect(m_intWatcher, &QFutureWatcher<int>::canceled, this, &QtParallelMapWidget::onProcessingCanceled);
    connect(m_intWatcher, &QFutureWatcher<int>::progressValueChanged, this, &QtParallelMapWidget::onProgressValueChanged);
    connect(m_intWatcher, &QFutureWatcher<int>::resultReadyAt, this, &QtParallelMapWidget::onResultReadyAt);
    
    connect(m_stringWatcher, &QFutureWatcher<QString>::finished, this, &QtParallelMapWidget::onProcessingFinished);
    connect(m_stringWatcher, &QFutureWatcher<QString>::canceled, this, &QtParallelMapWidget::onProcessingCanceled);
    connect(m_stringWatcher, &QFutureWatcher<QString>::progressValueChanged, this, &QtParallelMapWidget::onProgressValueChanged);
}

void QtParallelMapWidget::onLoadData()
{
    m_listResults->clear();
    m_dataIntegers.clear();
    m_dataStrings.clear();
    
    // 生成整数数据 (100个)
    for (int i = 1; i <= 100; ++i) {
        m_dataIntegers.append(i);
    }
    
    // 生成字符串数据 (50个)
    QStringList prefixes = {"Item", "Data", "Record", "Entry", "Value"};
    for (int i = 1; i <= 50; ++i) {
        QString prefix = prefixes.at(i % prefixes.size());
        m_dataStrings.append(QString("%1_%2").arg(prefix).arg(i));
    }
    
    logMessage(QString("生成模拟数据完成 - 整数: %1项, 字符串: %2项")
               .arg(m_dataIntegers.size()).arg(m_dataStrings.size()));
    
    m_statusLabel->setText(QString("数据就绪 - 可处理 %1 个整数或 %2 个字符串")
                          .arg(m_dataIntegers.size()).arg(m_dataStrings.size()));
    m_btnProcess->setEnabled(true);
    m_btnCancel->setEnabled(false);
    m_progressBar->setValue(0);
}

void QtParallelMapWidget::onProcessClicked()
{
    if (m_isProcessing) return;
    
    m_isProcessing = true;
    m_timer.start();
    
    logMessage("开始并行处理...");
    m_statusLabel->setText("正在并行处理数据...");
    m_listResults->clear();
    
    m_btnProcess->setEnabled(false);
    m_btnCancel->setEnabled(true);
    m_btnLoadData->setEnabled(false);
    m_btnClear->setEnabled(false);
    m_progressBar->setValue(0);
    
    // 随机选择处理类型
    bool processIntegers = QRandomGenerator::global()->bounded(2) == 0;
    
    // =========================================================================
    // 知识点：QtConcurrent::mapped 并行计算
    // =========================================================================
    // 1. 使用场景：
    //    当有一个容器（如 QList, QVector）包含大量数据，且每个数据的处理是独立的，
    //    可以使用 mapped 自动将任务分发到所有可用的 CPU 核心上并行执行。
    //    典型场景：图像批量处理、文件批量转换、大规模数据计算。
    //
    // 2. QFuture<T>：
    //    代表"未来会产生的结果"。它是一个轻量级的句柄，用于获取计算结果、状态等。
    //    注意：直接调用 future.results() 会阻塞当前线程，直到计算完成。
    //    要想不阻塞 UI，必须配合 QFutureWatcher 使用。
    //
    // 3. QtConcurrent::mapped vs map：
    //    - map(容器, 函数): 直接在原容器上修改数据（原地修改），函数返回 void。
    //    - mapped(容器, 函数): 原容器不变，返回包含新结果的新容器，函数返回新类型。
    // =========================================================================

    if (processIntegers && !m_dataIntegers.isEmpty()) {
        logMessage(QString("使用 QtConcurrent::mapped 处理 %1 个整数...").arg(m_dataIntegers.size()));
        
        // 启动并行计算：输入是 m_dataIntegers，处理函数是 processDataItem
        QFuture<int> future = QtConcurrent::mapped(m_dataIntegers, processDataItem);
        
        // 将 Future 交给 Watcher 监控，这样我们就能通过信号槽收到通知，而不用阻塞界面
        m_intWatcher->setFuture(future);
    } else if (!m_dataStrings.isEmpty()) {
        logMessage(QString("使用 QtConcurrent::mapped 处理 %1 个字符串...").arg(m_dataStrings.size()));
        
        // 启动并行计算：输入是 m_dataStrings，处理函数是 processStringItem
        QFuture<QString> future = QtConcurrent::mapped(m_dataStrings, processStringItem);
        
        // 监控字符串处理任务
        m_stringWatcher->setFuture(future);
    } else {
        logMessage("错误: 没有可用的数据进行处理");
        m_isProcessing = false;
        m_btnProcess->setEnabled(true);
        m_btnCancel->setEnabled(false);
        m_btnLoadData->setEnabled(true);
        m_btnClear->setEnabled(true);
        m_statusLabel->setText("错误: 无数据");
    }
}

void QtParallelMapWidget::onCancelClicked()
{
    if (!m_isProcessing) return;
    
    logMessage("正在请求取消任务...");
    m_statusLabel->setText("正在取消...");
    
    if (m_intWatcher->isRunning()) {
        // 取消任务：注意这只是发出取消请求，正在运行的线程可能会执行完当前项
        m_intWatcher->cancel();
    }
    if (m_stringWatcher->isRunning()) {
        m_stringWatcher->cancel();
    }
}

// ==================== FutureWatcher 槽函数 ====================

void QtParallelMapWidget::onProcessingFinished()
{
    qint64 elapsed = m_timer.elapsed();
    
    if (sender() == m_intWatcher) {
        QFuture<int> future = m_intWatcher->future();
        int resultCount = future.resultCount();
        
        logMessage(QString("整数处理完成! 耗时: %1ms, 处理项数: %2")
                   .arg(elapsed).arg(resultCount));
        
        // 显示部分结果
        QList<int> results = future.results();
        int displayCount = qMin(10, results.size());
        for (int i = 0; i < displayCount; ++i) {
            m_listResults->addItem(QString("结果[%1]: %2").arg(i+1).arg(results[i]));
        }
        if (results.size() > 10) {
            m_listResults->addItem(QString("... 还有 %1 项结果").arg(results.size() - 10));
        }
        
    } else if (sender() == m_stringWatcher) {
        QFuture<QString> future = m_stringWatcher->future();
        int resultCount = future.resultCount();
        
        logMessage(QString("字符串处理完成! 耗时: %1ms, 处理项数: %2")
                   .arg(elapsed).arg(resultCount));
        
        // 显示部分结果
        QStringList results = future.results();
        int displayCount = qMin(10, results.size());
        for (int i = 0; i < displayCount; ++i) {
            m_listResults->addItem(QString("结果[%1]: %2").arg(i+1).arg(results[i]));
        }
        if (results.size() > 10) {
            m_listResults->addItem(QString("... 还有 %1 项结果").arg(results.size() - 10));
        }
    }
    
    cleanupProcessingState();
}

void QtParallelMapWidget::onProgressValueChanged(int value)
{
    m_progressBar->setValue(value);
    // 每20%记录一次日志，避免过于频繁
    if (value % 20 == 0 || value == 100) {
        logMessage(QString("处理进度: %1%").arg(value));
    }
}

void QtParallelMapWidget::onResultReadyAt(int index)
{
    // 可以在这里实时显示单个结果
    // 为了避免UI更新过于频繁，这里暂不处理
    Q_UNUSED(index);
}

void QtParallelMapWidget::onProcessingCanceled()
{
    logMessage("任务已被取消");
    m_statusLabel->setText("任务已取消");
    cleanupProcessingState();
}

void QtParallelMapWidget::clearResults()
{
    m_listResults->clear();
    m_logViewer->clear();
    m_progressBar->setValue(0);
    m_statusLabel->setText("就绪 - 请点击生成数据");
    logMessage("已清空所有结果和日志");
}

// ==================== 辅助函数 ====================

void QtParallelMapWidget::cleanupProcessingState()
{
    m_isProcessing = false;
    m_btnProcess->setEnabled(true);
    m_btnCancel->setEnabled(false);
    m_btnLoadData->setEnabled(true);
    m_btnClear->setEnabled(true);
    m_progressBar->setValue(0);
    
    if (m_listResults->count() == 0) {
        m_statusLabel->setText("处理完成 - 无结果");
    } else {
        m_statusLabel->setText("处理完成 - 查看结果");
    }
}

// ==================== 静态处理函数 ====================

int QtParallelMapWidget::processDataItem(const int &item)
{
    // 模拟耗时计算
    QThread::msleep(10); // 每项处理10ms
    
    // 复杂的数学运算
    int result = item;
    for (int i = 0; i < 1000; ++i) {
        result = (result * 17 + 23) % 1000000;
    }
    
    return result;
}

QString QtParallelMapWidget::processStringItem(const QString &item)
{
    // 模拟字符串处理
    QThread::msleep(15); // 每项处理15ms
    
    // 字符串变换示例
    QString result = item.toUpper();
    result.replace("_", "-");
    
    // 添加处理标识
    result.prepend("PROCESSED_");
    
    return result;
}

void QtParallelMapWidget::logMessage(const QString &msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    m_logViewer->append(QString("[%1] %2").arg(timestamp, msg));
    
    // 自动滚动到底部
    QTextCursor cursor = m_logViewer->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logViewer->setTextCursor(cursor);
}
