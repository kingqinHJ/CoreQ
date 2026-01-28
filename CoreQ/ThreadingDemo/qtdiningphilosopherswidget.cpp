#include "qtdiningphilosopherswidget.h"
#include <QDateTime>
#include <QGroupBox>

QtDiningPhilosophersWidget::QtDiningPhilosophersWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void QtDiningPhilosophersWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *ctrlLayout = new QHBoxLayout();
    m_btnStart = new QPushButton("开始进餐", this);
    m_btnStop = new QPushButton("停止", this);
    m_btnStop->setEnabled(false);
    m_chkResolveDeadlock = new QCheckBox("启用死锁预防 (资源分级)", this);
    m_chkResolveDeadlock->setChecked(true);
    
    ctrlLayout->addWidget(m_btnStart);
    ctrlLayout->addWidget(m_btnStop);
    ctrlLayout->addWidget(m_chkResolveDeadlock);
    mainLayout->addLayout(ctrlLayout);

    // 哲学家状态展示
    QGroupBox *grpTable = new QGroupBox("餐桌状态", this);
    QHBoxLayout *tableLayout = new QHBoxLayout(grpTable);
    
    for(int i=0; i<5; ++i) {
        QLabel *lbl = new QLabel(QString("哲学家 %1\n[思考]").arg(i+1), this);
        lbl->setFrameStyle(QFrame::Box);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setFixedSize(80, 80);
        lbl->setStyleSheet("background-color: lightblue; border: 1px solid black;");
        m_philosophers.append(lbl);
        tableLayout->addWidget(lbl);
    }
    mainLayout->addWidget(grpTable);

    m_logViewer = new QTextEdit(this);
    m_logViewer->setReadOnly(true);
    mainLayout->addWidget(m_logViewer);

    connect(m_btnStart, &QPushButton::clicked, this, &QtDiningPhilosophersWidget::onStartClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &QtDiningPhilosophersWidget::onStopClicked);
}

void QtDiningPhilosophersWidget::onStartClicked()
{
    logMessage("哲学家开始就餐...");
    if (m_chkResolveDeadlock->isChecked()) {
        logMessage("策略: 启用死锁预防");
    } else {
        logMessage("策略: 只有随机等待，可能发生死锁");
    }
    m_btnStart->setEnabled(false);
    m_btnStop->setEnabled(true);
    m_chkResolveDeadlock->setEnabled(false);
    
    // TODO: 启动5个线程
}

void QtDiningPhilosophersWidget::onStopClicked()
{
    logMessage("停止就餐...");
    m_btnStart->setEnabled(true);
    m_btnStop->setEnabled(false);
    m_chkResolveDeadlock->setEnabled(true);
}

void QtDiningPhilosophersWidget::logMessage(const QString &msg)
{
    m_logViewer->append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), msg));
}
