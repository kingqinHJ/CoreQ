#include "qpromise.h"
#include <random>
#include <iostream>
#include<string>

QPromise::QPromise(QWidget *parent)
    : QWidget(parent)
    , startButton(new QPushButton("start calculate", this))
    , syncButton(new QPushButton(u8"同步获取结果", this))
    , progressBar(new QProgressBar(this))
    , resultLabel(new QLabel("waiting calculate...", this))
    , mainLayout(new QVBoxLayout(this))
    , progressTimer(new QTimer(this))
    , isComputing(false)
{
    setupUI();
    initConnections();
}

QPromise::~QPromise()
{
    if (computeThread && computeThread->joinable()) {
        computeThread->join();
    }
}

void QPromise::setupUI()
{
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    resultLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(new QLabel(u8"std::promise 示例", this));
    mainLayout->addWidget(startButton);
    mainLayout->addWidget(syncButton);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(resultLabel);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void QPromise::initConnections()
{
    connect(startButton, &QPushButton::clicked, this, &QPromise::startComputation);
    connect(syncButton, &QPushButton::clicked, this, &QPromise::startComputationSync);
    connect(this, &QPromise::progressUpdated, progressBar, &QProgressBar::setValue);
}


/**
 * @brief 后台计算函数（在独立线程运行）
 *
 * 说明：
 * - 使用随机数进行CPU密集型迭代，并周期性通过 progressUpdated(int) 发出进度信号；
 * - 迭代结束后调用 promise.set_value(result) 设置最终结果；
 * - 该函数始终在 computeThread 所代表的后台线程内运行，不触及主线程事件循环。
 */
void QPromise::compute(std::promise<int>&& p)
{
    //使用std库创建随机数
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    long long sum = 0;
    const int iterations = 10000000;

    //进行迭代运算
    for (int i = 0; i < iterations; ++i) {
        double randomNumber = dis(gen);
        sum += static_cast<long long>(randomNumber * randomNumber * 1000);

        if (i % (iterations / 100) == 0) {
            int progress = (i * 100) / iterations;
            emit progressUpdated(progress);
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    int result = static_cast<int>(sum / 1000);
    p.set_value(result);
}

/**
 * @brief 开始异步计算（非阻塞演示）
 *
 * 行为说明：
 * - 创建 promise/future 对并启动后台线程执行 compute(...)；
 * - 使用 QTimer::singleShot 定时调用 checkResult()，仅做状态轮询，不长时间等待；
 * - UI 始终保持响应，避免主线程被长时间阻塞。
 */
void QPromise::startComputation()
{
    if (isComputing) {
        return;
    }

    isComputing = true;
    startButton->setEnabled(false);
    progressBar->setValue(0);
    resultLabel->setText(u8"正在计算...");

    promisePtr = std::make_unique<std::promise<int>>();
    futurePtr = std::make_unique<std::future<int>>(promisePtr->get_future());

    computeThread = std::make_unique<std::thread>(&QPromise::compute, this, std::move(*promisePtr));           //启动线程

    QTimer::singleShot(50, this, &QPromise::checkResult);
}

/**
 * @brief 检查计算结果（异步轮询）
 *
 * 设计要点与原因：
 * - 后台线程计算完成后通过 std::promise 设置结果；
 * - 主线程使用 QTimer 定时触发本函数，采用 `future.wait_for(50ms)` 检查就绪状态；
 * - 仅当状态为 ready 时才调用 `future.get()` 获取结果，因此 `get()` 调用本身不会阻塞主线程；
 * - 注意：`wait_for(50ms)` 是一个短暂等待，严格意义上并非“零阻塞”；如果希望完全避免等待，可以改用 `wait_for(0ms)` 做纯检查，或在后台线程结束时使用 `QMetaObject::invokeMethod(..., Qt::QueuedConnection)` 将结果回投主线程。
 * - `std::future::get()` 只能调用一次，获取后立即释放相关资源。
 */
void QPromise::checkResult()
{
    if (!futurePtr) {
        return;
    }

    // 使用 wait_for 检查 future 的状态，等待 50 毫秒以避免阻塞主线程
    std::future_status status = futurePtr->wait_for(std::chrono::milliseconds(50));
    if (status == std::future_status::ready) {
        int result = futurePtr->get();
        resultLabel->setText(QString(u8"计算结果: %1").arg(result));
        progressBar->setValue(100);
        startButton->setEnabled(true);
        isComputing = false;

        if (computeThread && computeThread->joinable()) {
            computeThread->join();
        }

        promisePtr.reset();
        futurePtr.reset();
        computeThread.reset();
    } else {
        QTimer::singleShot(50, this, &QPromise::checkResult);
    }
}

/**
 * @brief 同步方式演示：阻塞式获取结果
 *
 * 使用 `std::future::get()` 在主线程直接等待后台线程设置值，演示同步（阻塞）用法：
 * - 主线程会阻塞直至 `promise.set_value(...)` 被调用；
 * - 阻塞期间事件循环暂停，进度条/界面更新不可见（即使后台线程发出进度信号，主线程无法处理事件）；
 * - 适用于需要严格串行语义的场景，但会降低程序交互响应性。
 */
void QPromise::startComputationSync()
{
    if (isComputing) {
        return;
    }

    isComputing = true;
    startButton->setEnabled(false);
    syncButton->setEnabled(false);
    progressBar->setValue(0);
    resultLabel->setText(u8"同步计算演示：主线程将阻塞，直到结果就绪...");

    // 建立 promise/future 并启动后台线程
    promisePtr = std::make_unique<std::promise<int>>();
    futurePtr = std::make_unique<std::future<int>>(promisePtr->get_future());
    computeThread = std::make_unique<std::thread>(&QPromise::compute, this, std::move(*promisePtr));

    // 同步阻塞：直接在主线程等待结果
    int result = futurePtr->get();

    // 获取到结果后不要立即直接更新 UI：
    // 问题说明：在同步阻塞期间（future.get()），后台线程发出的进度信号(0..99)会积压到主线程事件队列。
    // 解除阻塞后，这些排队的事件将按序处理，可能覆盖我们刚设置的“最终结果/进度100”，导致看起来未生效或回到99。
    // 正确做法：使用一次排队调用，将“最终设置 100 和结果文本”的更新置于队列末尾，确保最后执行，不被历史事件覆盖。不要直接调用
    // resultLabel->setText(QString(u8"同步计算结果: %1").arg(result));progressBar->setValue(100);
    QMetaObject::invokeMethod(this, [this, result] {
        resultLabel->setText(QString(u8"同步计算结果: %1").arg(result));
        progressBar->setValue(100);
    }, Qt::QueuedConnection);
    startButton->setEnabled(true);
    syncButton->setEnabled(true);
    isComputing = false;

    if (computeThread && computeThread->joinable()) {
        computeThread->join();
    }

    promisePtr.reset();
    futurePtr.reset();
    computeThread.reset();
}
