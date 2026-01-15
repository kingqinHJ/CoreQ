#ifndef QTTHREADBASICSWIDGET_H
#define QTTHREADBASICSWIDGET_H

#include <QWidget>
#include <QThread>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QGroupBox>

// ==========================================
// 1. Worker-Object 模式 (推荐)
// ==========================================
class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void doWork(const QString &parameter);

signals:
    void resultReady(const QString &result);
    void workFinished();
};

// ==========================================
// 2. QThread 子类化模式 (传统/特定场景)
// ==========================================
class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = nullptr) : QThread(parent) {}
    void run() override;

signals:
    void resultReady(const QString &result);
};

// ==========================================
// 主演示窗口
// ==========================================
class QtThreadBasicsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtThreadBasicsWidget(QWidget *parent = nullptr);
    ~QtThreadBasicsWidget();

private slots:
    // Worker模式槽函数
    void startWorker();
    void stopWorker();
    void handleWorkerResult(const QString &result);
    void handleWorkerFinished();

    // 子类化模式槽函数
    void startSubclassThread();
    void handleSubclassResult(const QString &result);

    void clearLog();

private:
    void logMessage(const QString &msg);

private:
    // UI Controls
    QPushButton *m_btnStartWorker;
    QPushButton *m_btnStopWorker;
    QPushButton *m_btnStartSubclass;
    QTextEdit *m_logDisplay;

    // Worker Pattern Members
    QThread *m_workerThread;
    Worker *m_worker;

    // Subclass Pattern Members
    MyThread *m_subclassThread;
};

#endif // QTTHREADBASICSWIDGET_H
