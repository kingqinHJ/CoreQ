#ifndef CONDITIONVARIABLEWIDGET_H
#define CONDITIONVARIABLEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QSpinBox>
#include <QGroupBox>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <atomic>

/**
 * @class ConditionVariableWidget
 * @brief std::condition_variable 演示类 - 生产者-消费者模型
 * 
 * 这个类演示了条件变量的典型应用场景：
 * 1. wait(): 消费者等待数据
 * 2. notify_one() / notify_all(): 生产者唤醒消费者
 * 3. unique_lock: 配合条件变量使用的锁
 */
class ConditionVariableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConditionVariableWidget(QWidget *parent = nullptr);
    ~ConditionVariableWidget();

private slots:
    void startProducerConsumer();
    void stopAll();
    void clearLog();
    void updateUI(); // 定时更新UI（避免跨线程直接操作UI）

private:
    void initUI();
    void producerThread();
    void consumerThread(int id);
    void logMessage(const QString &msg);

private:
    // UI Controls
    QSpinBox *m_producerSpeed;
    QSpinBox *m_consumerSpeed;
    QSpinBox *m_consumerCount;
    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;
    QTextEdit *m_logDisplay;
    QProgressBar *m_bufferBar;
    QLabel *m_statusLabel;
    
    // Threading members
    std::mutex m_mutex;
    std::condition_variable m_cv_not_empty; // 缓冲区非空条件（消费者等待）
    std::condition_variable m_cv_not_full;  // 缓冲区非满条件（生产者等待）
    
    std::queue<int> m_buffer;
    const size_t MAX_BUFFER_SIZE = 10;
    
    std::vector<std::thread> m_threads;
    std::atomic<bool> m_running{false};
    std::atomic<int> m_producedCount{0};
    std::atomic<int> m_consumedCount{0};
    
    // UI Update buffer
    std::mutex m_logMutex;
    QString m_pendingLogs;
    QTimer *m_uiTimer;
};

#endif // CONDITIONVARIABLEWIDGET_H
