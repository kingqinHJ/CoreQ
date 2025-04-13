#ifndef QPROMISE_H
#define QPROMISE_H

#include <QtWidgets>
#include <future>
#include <thread>

class QPromise : public QWidget
{
    Q_OBJECT
public:
    explicit QPromise(QWidget *parent = nullptr);
    ~QPromise();

private slots:
    void startComputation();
    void checkResult();

signals:
    void progressUpdated(int value);  // 新增信号，用于更新进度

private:
    void setupUI();
    void initConnections();
    void compute(std::promise<int>&& p);

    QPushButton *startButton;
    QProgressBar *progressBar;
    QLabel *resultLabel;
    QVBoxLayout *mainLayout;

    std::unique_ptr<std::promise<int>> promisePtr;
    std::unique_ptr<std::future<int>> futurePtr;
    std::unique_ptr<std::thread> computeThread;
    QTimer *progressTimer;
    
    bool isComputing;
};

#endif // QPROMISE_H
