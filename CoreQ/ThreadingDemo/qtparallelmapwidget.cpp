#include "qtparallelmapwidget.h"
#include <QDateTime>
#include <QGroupBox>

QtParallelMapWidget::QtParallelMapWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void QtParallelMapWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. 操作栏
    QHBoxLayout *topLayout = new QHBoxLayout();
    m_btnLoadData = new QPushButton("1. 生成模拟数据", this);
    m_btnProcess = new QPushButton("2. 并行处理 (Map)", this);
    m_btnCancel = new QPushButton("取消任务", this);
    m_btnProcess->setEnabled(false);
    m_btnCancel->setEnabled(false);

    topLayout->addWidget(m_btnLoadData);
    topLayout->addWidget(m_btnProcess);
    topLayout->addWidget(m_btnCancel);
    mainLayout->addLayout(topLayout);

    // 2. 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setValue(0);
    mainLayout->addWidget(m_progressBar);

    // 3. 结果显示
    QGroupBox *grpResults = new QGroupBox("处理结果", this);
    QVBoxLayout *resLayout = new QVBoxLayout(grpResults);
    m_listResults = new QListWidget(this);
    resLayout->addWidget(m_listResults);
    mainLayout->addWidget(grpResults);

    // 4. 日志
    m_logViewer = new QTextEdit(this);
    m_logViewer->setMaximumHeight(150);
    m_logViewer->setReadOnly(true);
    mainLayout->addWidget(m_logViewer);

    connect(m_btnLoadData, &QPushButton::clicked, this, &QtParallelMapWidget::onLoadData);
    connect(m_btnProcess, &QPushButton::clicked, this, &QtParallelMapWidget::onProcessClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &QtParallelMapWidget::onCancelClicked);
}

void QtParallelMapWidget::onLoadData()
{
    m_listResults->clear();
    logMessage("生成了 100 个待处理项...");
    // TODO: 填充一个 QList<int> 或 QList<Image> 数据
    m_btnProcess->setEnabled(true);
}

void QtParallelMapWidget::onProcessClicked()
{
    logMessage("开始并行处理...");
    m_btnProcess->setEnabled(false);
    m_btnCancel->setEnabled(true);
    m_btnLoadData->setEnabled(false);
    m_progressBar->setValue(0);
    
    // TODO: 使用 QtConcurrent::map 或 mapped
    // TODO: 使用 QFutureWatcher 监控进度 (progressValueChanged) 和完成状态 (finished)
}

void QtParallelMapWidget::onCancelClicked()
{
    logMessage("尝试取消任务...");
    // TODO: 调用 watcher.cancel() 或 future.cancel()
}

void QtParallelMapWidget::logMessage(const QString &msg)
{
    m_logViewer->append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), msg));
}
