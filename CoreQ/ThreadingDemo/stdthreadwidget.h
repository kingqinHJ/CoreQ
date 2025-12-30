#ifndef STDTHREADWIDGET_H
#define STDTHREADWIDGET_H

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
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <mutex>
#include <future>
/**
 * @class StdThreadWidget
 * @brief std::thread 演示类 - C++11标准线程库的使用示例
 * 
 * 这个类演示了 std::thread 的各种使用方法，包括：
 * 1. 基本线程创建和管理
 * 2. 线程间数据共享和同步
 * 3. 多线程并发执行
 * 4. 线程安全的数据访问
 * 5. 线程的生命周期管理
 * 
 * std::thread 核心概念：
 * - std::thread: C++11引入的标准线程类，用于创建和管理线程
 * - std::atomic: 原子操作类型，保证线程安全的数据访问
 * - std::mutex: 互斥锁，用于保护共享资源
 * - join(): 等待线程完成执行
 * - detach(): 分离线程，让其在后台独立运行
 * 
 * 应用场景：
 * - CPU密集型任务的并行处理
 * - 后台任务执行
 * - 多任务并发处理
 * - 提高程序响应性
 * 
 * 优势：
 * - 标准库支持，跨平台兼容
 * - 轻量级线程管理
 * - 与现代C++特性良好集成
 * - 丰富的同步原语支持
 */
class StdThreadWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit StdThreadWidget(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数 - 确保所有线程正确清理
     */
    ~StdThreadWidget();

private slots:
    /**
     * @brief 启动单个线程演示
     */
    void startSingleThread();
    
    /**
     * @brief 启动多线程演示
     */
    void startMultipleThreads();
    
    /**
     * @brief 停止所有线程
     */
    void stopAllThreads();
    
    /**
     * @brief 清空日志显示
     */
    void clearLog();
    
    /**
     * @brief 更新UI显示（通过定时器调用）
     */
    void updateUI();

    /**
    * @brief 启动异步扩展演示，使用 std::async 和错误处理
    */
    void startAsyncExtension();

private:
    /**
     * @brief 初始化用户界面
     */
    void initUI();
    
    /**
     * @brief 单线程工作函数
     * @param threadId 线程标识符
     * @param workTime 工作时间（秒）
     */
    void singleThreadWork(int threadId, int workTime);
    
    /**
     * @brief 多线程工作函数
     * @param threadId 线程标识符
     * @param iterations 迭代次数
     */
    void multiThreadWork(int threadId, int iterations);
    
    /**
     * @brief 线程安全的日志添加函数
     * @param message 日志消息
     */
    void addLogSafe(const QString& message);
    
    /**
     * @brief 等待所有线程完成
     */
    void joinAllThreads();
    
private:
    // UI组件
    QPushButton* m_startSingleBtn;      ///< 启动单线程按钮
    QPushButton* m_startMultiBtn;       ///< 启动多线程按钮
    QPushButton* m_stopBtn;             ///< 停止按钮
    QPushButton* m_clearBtn;            ///< 清空日志按钮
    
    QSpinBox* m_singleWorkTime;         ///< 单线程工作时间设置
    QSpinBox* m_threadCount;            ///< 线程数量设置
    QSpinBox* m_iterations;             ///< 迭代次数设置
    
    QProgressBar* m_progressBar;        ///< 进度条
    QLabel* m_statusLabel;              ///< 状态标签
    QTextEdit* m_logDisplay;            ///< 日志显示区域
    
    QGroupBox* m_singleThreadGroup;     ///< 单线程演示组
    QGroupBox* m_multiThreadGroup;      ///< 多线程演示组
    QGroupBox* m_logGroup;              ///< 日志显示组
    
    // 线程管理
    std::vector<std::thread> m_threads; ///< 线程容器
    std::atomic<bool> m_stopFlag;       ///< 停止标志（原子操作）
    std::atomic<int> m_completedTasks;  ///< 已完成任务数（原子操作）
    std::atomic<int> m_totalTasks;      ///< 总任务数（原子操作）
    
    // 线程同步
    std::mutex m_logMutex;              ///< 日志互斥锁
    std::mutex m_uiMutex;               ///< UI更新互斥锁
    
    // UI更新
    QTimer* m_updateTimer;              ///< UI更新定时器
    
    // 状态管理
    bool m_isRunning;                   ///< 是否正在运行
    QString m_pendingLogs;              ///< 待显示的日志

    QPushButton* m_startAsyncBtn;       ///< 异步启动按钮
    QLabel* m_errorLogLabel;            ///< 错误日志标签
    std::future<int> m_asyncFuture;     ///< 异步操作结果
};

#endif // STDTHREADWIDGET_H