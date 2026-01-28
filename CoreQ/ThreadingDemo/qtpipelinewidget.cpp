#include "qtpipelinewidget.h"
#include <QDateTime>

QtPipelineWidget::QtPipelineWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
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
    QGroupBox *grpPipeline = new QGroupBox("流水线状态 (Stage A -> Stage B -> Stage C)", this);
    QHBoxLayout *pipeLayout = new QHBoxLayout(grpPipeline);
    
    auto createStageLabel = [this](const QString &text) {
        QLabel *lbl = new QLabel(text, this);
        lbl->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setFixedSize(100, 60);
        lbl->setStyleSheet("background-color: lightgray;");
        return lbl;
    };

    m_lblStage1 = createStageLabel("Stage 1\n(采集)");
    m_lblStage2 = createStageLabel("Stage 2\n(处理)");
    m_lblStage3 = createStageLabel("Stage 3\n(存储)");

    pipeLayout->addWidget(m_lblStage1);
    pipeLayout->addWidget(new QLabel("-->"));
    pipeLayout->addWidget(m_lblStage2);
    pipeLayout->addWidget(new QLabel("-->"));
    pipeLayout->addWidget(m_lblStage3);
    
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

void QtPipelineWidget::onStartClicked()
{
    logMessage("启动流水线...");
    m_btnStart->setEnabled(false);
    m_btnStop->setEnabled(true);
    // TODO: 启动3个线程，通过信号槽或线程安全队列传递数据
}

void QtPipelineWidget::onStopClicked()
{
    logMessage("停止流水线...");
    m_btnStart->setEnabled(true);
    m_btnStop->setEnabled(false);
}

void QtPipelineWidget::logMessage(const QString &msg)
{
    m_logViewer->append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), msg));
}
