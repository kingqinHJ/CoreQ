#include "qpromise.h"
#include <random>
#include <iostream>

QPromise::QPromise(QWidget *parent)
    : QWidget(parent)
    , startButton(new QPushButton("start calculate", this))
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

    mainLayout->addWidget(new QLabel("std::promise 示例", this));
    mainLayout->addWidget(startButton);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(resultLabel);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void QPromise::initConnections()
{
    connect(startButton, &QPushButton::clicked, this, &QPromise::startComputation);
    connect(this, &QPromise::progressUpdated, progressBar, &QProgressBar::setValue);
}


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

void QPromise::startComputation()
{
    if (isComputing) {
        return;
    }

    isComputing = true;
    startButton->setEnabled(false);
    progressBar->setValue(0);
    resultLabel->setText("正在计算...");

    promisePtr = std::make_unique<std::promise<int>>();
    futurePtr = std::make_unique<std::future<int>>(promisePtr->get_future());

    computeThread = std::make_unique<std::thread>(&QPromise::compute, this, std::move(*promisePtr));           //启动线程

    QTimer::singleShot(50, this, &QPromise::checkResult);
}

void QPromise::checkResult()
{
    if (!futurePtr) {
        return;
    }

    // 使用 wait_for 检查 future 的状态，等待 50 毫秒以避免阻塞主线程
    std::future_status status = futurePtr->wait_for(std::chrono::milliseconds(50));
    if (status == std::future_status::ready) {
        int result = futurePtr->get();
        resultLabel->setText(QString("计算结果: %1").arg(result));
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
