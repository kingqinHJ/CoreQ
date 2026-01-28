#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include<QThread>
#include<QWaitCondition>
#include<QMutex>
#include<QVector>



class BufferController : public QObject
{
    Q_OBJECT
public:
    explicit BufferController(int maxsize,QObject *parent = nullptr):
    QObject(parent),m_maxSize(maxsize)
    {
        
    }

    // 生产数据（供生产者调用）
    void produce(int data);
    // 消费数据（供消费者调用）
    int consume();

    // 设置最大容量
    void setMaxSize(int size) { m_maxSize = size; }

    // 停止并唤醒所有等待线程
    void stop();

signals:
    // 通知 UI 更新的信号
    void bufferUpdated(const QVector<int> &currentBuffer);
    void logRequest(const QString &msg);

private:
    QVector<int> m_buffer;
    int m_maxSize;
    bool m_stop = false; // 停止标志
    QMutex m_mutex;
    QWaitCondition m_bufferNotFull;  // 条件变量：缓冲区不满（可以生产）
    QWaitCondition m_bufferNotEmpty; // 条件变量：缓冲区不空（可以消费）
};

class ProducerThread : public QThread
{
    Q_OBJECT
public:
    explicit ProducerThread(BufferController *controller,int interval, QObject *parent = nullptr)
            :QThread(parent),m_controller(controller),m_interval(interval)
    {
        
    }

    void setInterval(int interval) { m_interval = interval; }
    void stop() { m_running = false; }


protected:
    void run() override;

private:
    BufferController *m_controller;
    int m_interval; // 生产间隔（毫秒）
    bool m_running = true; // 运行标志
};


class ConsumerThread : public QThread
{
    Q_OBJECT
public:
    explicit ConsumerThread(BufferController *controller,int interval, QObject *parent = nullptr)
            :QThread(parent),m_controller(controller),m_interval(interval)
    {
        
    }

    void setInterval(int interval) { m_interval = interval; }
    void stop() { m_running = false; }


protected:
    void run() override;

private:
    BufferController *m_controller;
    int m_interval; // 生产间隔（毫秒）
    bool m_running = true; // 运行标志
};

class QtProducerConsumerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtProducerConsumerWidget(QWidget *parent = nullptr);

private slots:
    void onStartClicked();
    void onStopClicked();
    void onClearLogClicked();

    void onThreadFinished();
    void logMessage(const QString &msg);

private:
    void setupUi();
    

    // UI 控件
    QSpinBox *m_spinBufferSize;
    QSpinBox *m_spinProduceSpeed; // 毫秒
    QSpinBox *m_spinConsumeSpeed; // 毫秒
    
    QPushButton *m_btnStart;
    QPushButton *m_btnStop;
    QPushButton *m_btnClearLog;

    QListWidget *m_listBuffer; // 可视化缓冲区
    QTextEdit *m_logViewer;
    QLabel *m_lblStatus;

    // 线程成员变量
    ProducerThread *m_producerThread=nullptr;
    ConsumerThread *m_consumerThread=nullptr;   
    BufferController *m_bufferController=nullptr;
};
