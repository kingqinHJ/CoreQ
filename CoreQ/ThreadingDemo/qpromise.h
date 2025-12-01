#ifndef QPROMISE_H
#define QPROMISE_H

#include <QtWidgets>
#include <future>
#include <thread>

/**
 * @brief QPromise类 - C++11 std::promise/std::future 异步编程演示
 * 
 * 这个类演示了如何使用C++11的promise/future机制来实现异步计算：
 * 
 * 核心概念：
 * - std::promise: 承诺对象，用于在一个线程中设置值
 * - std::future: 未来对象，用于在另一个线程中获取值
 * - 两者配对使用，实现线程间的单向数据传递
 * 
 * 应用场景：
 * - 异步计算：在后台线程执行耗时操作，主线程不阻塞
 * - 线程间通信：安全地在线程间传递计算结果
 * - 延迟获取：可以在需要时才获取计算结果
 * 
 * 优势：
 * - 类型安全：编译时确定数据类型
 * - 异常安全：可以传递异常信息
 * - 一次性使用：避免重复读取问题
 */
class QPromise : public QWidget
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit QPromise(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     * 确保线程正确结束，避免资源泄漏
     */
    ~QPromise();

private slots:
    /**
     * @brief 开始异步计算
     * 创建promise/future对，启动后台计算线程
     */
    void startComputation();
    
    /**
     * @brief 检查计算结果
     * 非阻塞方式检查future状态，获取计算结果
     */
    void checkResult();

    /**
     * @brief 同步方式演示：阻塞式获取结果
     * 使用 std::future::get() 直接在主线程等待后台线程设置结果，
     * 演示同步用法的行为特点与对 UI 响应性的影响。
     */
    void startComputationSync();

signals:
    /**
     * @brief 进度更新信号
     * @param value 当前进度值(0-100)
     * 用于从后台线程向主线程传递进度信息
     */
    void progressUpdated(int value);

private:
    /**
     * @brief 设置用户界面
     * 创建并布局所有UI控件
     */
    void setupUI();
    
    /**
     * @brief 初始化信号槽连接
     * 连接按钮点击、进度更新等信号槽
     */
    void initConnections();
    
    /**
     * @brief 后台计算函数
     * @param p promise对象的右值引用，用于设置计算结果
     * 
     * 在独立线程中执行：
     * 1. 生成随机数并进行复杂计算
     * 2. 定期发送进度更新信号
     * 3. 计算完成后通过promise设置结果
     */
    void compute(std::promise<int>&& p);

    // UI控件
    QPushButton *startButton;    ///< 开始计算按钮
    QPushButton *syncButton;     ///< 同步演示按钮（阻塞式获取结果）
    QProgressBar *progressBar;   ///< 进度条，显示计算进度
    QLabel *resultLabel;         ///< 结果标签，显示计算结果或状态
    QVBoxLayout *mainLayout;     ///< 主布局管理器

    // 异步编程核心对象
    std::unique_ptr<std::promise<int>> promisePtr;  ///< promise智能指针，用于设置计算结果
    std::unique_ptr<std::future<int>> futurePtr;    ///< future智能指针，用于获取计算结果
    std::unique_ptr<std::thread> computeThread;     ///< 计算线程智能指针
    
    QTimer *progressTimer;       ///< 定时器，用于定期检查计算状态（当前未使用）
    bool isComputing;           ///< 计算状态标志，防止重复启动计算
};

#endif // QPROMISE_H
