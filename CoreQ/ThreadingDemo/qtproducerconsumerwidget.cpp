#include "qtproducerconsumerwidget.h"
#include <QDateTime>
#include <QRandomGenerator>

void BufferController::produce(int data)
{
    QMutexLocker locker(&m_mutex);
    if(m_stop)
    {
        return;
    }

    while (m_buffer.size() >= m_maxSize)
    {
        if(m_stop)
        {
            return;
        }
        emit logRequest(QString("BufferController: 缓冲区已满，生产者等待"));
        m_bufferNotFull.wait(&m_mutex); // 等待缓冲区不满
        if(m_stop)
        {
            return;
        }
    }
    m_buffer.append(data);
    emit bufferUpdated(m_buffer);
    emit logRequest(QString("生产者 + 生产数据: %1 (当前数量: %2)").arg(data).arg(m_buffer.size()));
    m_bufferNotEmpty.wakeAll(); // 唤醒一个等待的消费者
}

int BufferController::consume()
{
    QMutexLocker locker(&m_mutex);
    if(m_stop)
    {
        return 0;
    }

    while (m_buffer.isEmpty())
    {
        if(m_stop)
        {
            return 0;
        }
        emit logRequest(QString("BufferController: 缓冲区为空，消费者等待"));
        m_bufferNotEmpty.wait(&m_mutex); // 等待缓冲区不空
        if(m_stop)
        {
            return 0;
        }
    }
    int data = m_buffer.takeFirst();
    emit bufferUpdated(m_buffer);
    emit logRequest(QString("消费者 - 消费数据: %1 (当前数量: %2)").arg(data).arg(m_buffer.size()));
    m_bufferNotFull.wakeAll(); // 唤醒一个等待的生产者
    return data;
}

void BufferController::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stop = true;
    emit logRequest(QString("BufferController: 发出停止信号，唤醒所有线程"));
    m_bufferNotFull.wakeAll();
    m_bufferNotEmpty.wakeAll();
}

void ProducerThread::run()
{
    m_running=true;
    while (m_running&&!isInterruptionRequested())
    {
        msleep(m_interval);

        int data=QRandomGenerator::global()->bounded(100,999);
        m_controller->produce(data);
    }
}

void ConsumerThread::run()
{
    m_running=true;
    while (m_running&&!isInterruptionRequested())
    {
        msleep(m_interval);

        int data=m_controller->consume();
    }
}

QtProducerConsumerWidget::QtProducerConsumerWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void QtProducerConsumerWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. 控制区
    QGroupBox *grpControl = new QGroupBox("控制面板", this);
    QHBoxLayout *controlLayout = new QHBoxLayout(grpControl);

    controlLayout->addWidget(new QLabel("缓冲区大小:"));
    m_spinBufferSize = new QSpinBox(this);
    m_spinBufferSize->setRange(1, 100);
    m_spinBufferSize->setValue(5);
    controlLayout->addWidget(m_spinBufferSize);

    controlLayout->addWidget(new QLabel("生产延时(ms):"));
    m_spinProduceSpeed = new QSpinBox(this);
    m_spinProduceSpeed->setRange(10, 2000);
    m_spinProduceSpeed->setValue(500);
    controlLayout->addWidget(m_spinProduceSpeed);

    controlLayout->addWidget(new QLabel("消费延时(ms):"));
    m_spinConsumeSpeed = new QSpinBox(this);
    m_spinConsumeSpeed->setRange(10, 2000);
    m_spinConsumeSpeed->setValue(800);
    controlLayout->addWidget(m_spinConsumeSpeed);

    m_btnStart = new QPushButton("开始", this);
    m_btnStop = new QPushButton("停止", this);
    m_btnStop->setEnabled(false);
    controlLayout->addWidget(m_btnStart);
    controlLayout->addWidget(m_btnStop);
    
    mainLayout->addWidget(grpControl);

    // 2. 缓冲区可视化
    QGroupBox *grpBuffer = new QGroupBox("缓冲区状态", this);
    QVBoxLayout *bufferLayout = new QVBoxLayout(grpBuffer);
    m_listBuffer = new QListWidget(this);
    m_listBuffer->setViewMode(QListWidget::IconMode); //以此模式显示更直观
    m_listBuffer->setFlow(QListWidget::LeftToRight);
    //m_listBuffer->setFixedHeight(100);
    bufferLayout->addWidget(m_listBuffer);
    mainLayout->addWidget(grpBuffer);

    // 3. 日志区
    QGroupBox *grpLog = new QGroupBox("运行日志", this);
    QVBoxLayout *logLayout = new QVBoxLayout(grpLog);
    m_logViewer = new QTextEdit(this);
    m_logViewer->setReadOnly(true);
    m_btnClearLog = new QPushButton("清空日志", this);
    
    logLayout->addWidget(m_logViewer);
    logLayout->addWidget(m_btnClearLog);
    mainLayout->addWidget(grpLog);

    // 4. 信号槽连接
    connect(m_btnStart, &QPushButton::clicked, this, &QtProducerConsumerWidget::onStartClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &QtProducerConsumerWidget::onStopClicked);
    connect(m_btnClearLog, &QPushButton::clicked, this, &QtProducerConsumerWidget::onClearLogClicked);
}

void QtProducerConsumerWidget::onStartClicked()
{
    logMessage(">>> 正在启动系统...");
    m_btnStart->setEnabled(false);
    m_btnStop->setEnabled(true);
    m_spinBufferSize->setEnabled(false);
    
    if(m_bufferController==nullptr)
    {
        delete m_bufferController;
        m_bufferController=new BufferController(m_spinBufferSize->value(),this);
    }

    connect(m_bufferController,&BufferController::logRequest,this,&QtProducerConsumerWidget::logMessage);

    if(m_producerThread==nullptr)
    {
        delete m_producerThread;
        m_producerThread=new ProducerThread(m_bufferController,m_spinProduceSpeed->value(),this);
        connect(m_producerThread,&ProducerThread::finished,this,&QtProducerConsumerWidget::onThreadFinished);
    }
    if(m_consumerThread==nullptr)
    {
        delete m_consumerThread;
        m_consumerThread=new ConsumerThread(m_bufferController,m_spinConsumeSpeed->value(),this);
        connect(m_consumerThread,&ConsumerThread::finished,this,&QtProducerConsumerWidget::onThreadFinished);
    }

    m_producerThread->start();
    m_consumerThread->start();
    logMessage("系统启动完成");
}

void QtProducerConsumerWidget::onStopClicked()
{
    logMessage("<<< 正在停止系统...");
    m_btnStart->setEnabled(true);
    m_btnStop->setEnabled(false);
    m_spinBufferSize->setEnabled(true);

    logMessage("请求停止生产者-消费者模型...");

    if(m_bufferController!=nullptr)
    {
        m_bufferController->stop();
    }
    if(m_producerThread!=nullptr)
    {
        m_producerThread->stop();
        m_producerThread->requestInterruption();
    }
    if(m_consumerThread!=nullptr)
    {
        m_consumerThread->stop();
        m_consumerThread->requestInterruption();
    }
}

void QtProducerConsumerWidget::onClearLogClicked()
{
    m_logViewer->clear();
}

void QtProducerConsumerWidget::logMessage(const QString &msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    m_logViewer->append(QString("[%1] %2").arg(timestamp, msg));
}

void QtProducerConsumerWidget::onThreadFinished()
{
    QThread *senderThread=qobject_cast<QThread*>(sender());
    if(!senderThread){
        return; 
    }
    if(senderThread==m_producerThread)
    {
        m_producerThread=nullptr;
        logMessage("生产者线程已完成");
    }
    else if(senderThread==m_consumerThread)
    {
        m_consumerThread=nullptr;
        logMessage("消费者线程已完成");
    }

    if(!m_producerThread && !m_consumerThread)
    {
        if(m_bufferController!=nullptr)
        {
            delete m_bufferController;
            m_bufferController=nullptr;
        }
        logMessage("系统已安全停止");
        m_btnStart->setEnabled(true);
        m_spinBufferSize->setEnabled(true);
    }
}