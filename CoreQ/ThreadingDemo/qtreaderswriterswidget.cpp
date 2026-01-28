#include "qtreaderswriterswidget.h"
#include <QDateTime>

QtReadersWritersWidget::QtReadersWritersWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void QtReadersWritersWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. 设置区
    QGroupBox *grpSettings = new QGroupBox("配置", this);
    QHBoxLayout *settingsLayout = new QHBoxLayout(grpSettings);
    
    settingsLayout->addWidget(new QLabel("读者数量:"));
    m_spinReaderCount = new QSpinBox(this);
    m_spinReaderCount->setRange(1, 10);
    m_spinReaderCount->setValue(3);
    settingsLayout->addWidget(m_spinReaderCount);

    settingsLayout->addWidget(new QLabel("写者数量:"));
    m_spinWriterCount = new QSpinBox(this);
    m_spinWriterCount->setRange(1, 5);
    m_spinWriterCount->setValue(1);
    settingsLayout->addWidget(m_spinWriterCount);

    m_btnStart = new QPushButton("启动模拟", this);
    m_btnStop = new QPushButton("停止模拟", this);
    m_btnStop->setEnabled(false);
    settingsLayout->addWidget(m_btnStart);
    settingsLayout->addWidget(m_btnStop);
    
    mainLayout->addWidget(grpSettings);

    // 2. 共享数据展示
    QGroupBox *grpData = new QGroupBox("共享资源", this);
    QVBoxLayout *dataLayout = new QVBoxLayout(grpData);
    m_lblSharedData = new QLabel("当前值: 0", this);
    QFont font = m_lblSharedData->font();
    font.setPointSize(20);
    font.setBold(true);
    m_lblSharedData->setFont(font);
    m_lblSharedData->setAlignment(Qt::AlignCenter);
    dataLayout->addWidget(m_lblSharedData);
    mainLayout->addWidget(grpData);

    // 3. 日志
    m_logViewer = new QTextEdit(this);
    m_logViewer->setReadOnly(true);
    mainLayout->addWidget(m_logViewer);

    connect(m_btnStart, &QPushButton::clicked, this, &QtReadersWritersWidget::onStartClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &QtReadersWritersWidget::onStopClicked);
}

void QtReadersWritersWidget::onStartClicked()
{
    logMessage("开始读写者模型演示...");
    m_btnStart->setEnabled(false);
    m_btnStop->setEnabled(true);
    // TODO: 创建并启动 m_spinReaderCount 个读线程和 m_spinWriterCount 个写线程
    // 提示：使用 QReadWriteLock 保护共享变量
}

void QtReadersWritersWidget::onStopClicked()
{
    logMessage("停止演示...");
    m_btnStart->setEnabled(true);
    m_btnStop->setEnabled(false);
    // TODO: 停止所有线程
}

void QtReadersWritersWidget::logMessage(const QString &msg)
{
    m_logViewer->append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), msg));
}
