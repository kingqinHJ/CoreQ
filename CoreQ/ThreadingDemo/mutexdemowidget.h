#ifndef MUTEXDEMOWIDGET_H
#define MUTEXDEMOWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QTimer>
#include <QSpinBox>
#include <QGroupBox>

/**
 * MutexDemoWidget - std::mutex 演示界面（仅UI与占位逻辑）
 * 
 * 目标：提供交互控制与展示区域，后续把并发逻辑（计数、读者-写者、死锁演示）由你手动敲入。
 */
class MutexDemoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MutexDemoWidget(QWidget* parent = nullptr);
    ~MutexDemoWidget();

private slots:
    // 基础计数演示：无锁/互斥/原子（占位）
    void startUnlockedCount();
    void startMutexCount();
    void startAtomicCount();

    // 读者-写者演示（占位）
    void startReadersWriters();

    // 死锁与避免演示（占位）
    void startDeadlockDemo();

    // 控制
    void stopAll();
    void clearLog();
    void updateUI();

private:
    void initUI();
    void addLogUnsafe(const QString& msg);

private:
    // 计数演示
    QGroupBox* m_counterGroup{};
    QSpinBox* m_threadCount{};
    QSpinBox* m_iterations{};
    QPushButton* m_startUnlockedBtn{};
    QPushButton* m_startMutexBtn{};
    QPushButton* m_startAtomicBtn{};

    // 读者-写者演示
    QGroupBox* m_rwGroup{};
    QSpinBox* m_readerThreads{};
    QSpinBox* m_writerThreads{};
    QPushButton* m_startRWBtn{};

    // 死锁演示
    QGroupBox* m_deadlockGroup{};
    QPushButton* m_startDeadlockBtn{};

    // 控制与展示
    QPushButton* m_stopBtn{};
    QPushButton* m_clearBtn{};
    QProgressBar* m_progressBar{};
    QLabel* m_statusLabel{};
    QTextEdit* m_logDisplay{};

    QTimer* m_updateTimer{};
    QString m_pendingLogs;
    bool m_isRunning{false};
    bool m_stopRequested{false};
};

#endif // MUTEXDEMOWIDGET_H