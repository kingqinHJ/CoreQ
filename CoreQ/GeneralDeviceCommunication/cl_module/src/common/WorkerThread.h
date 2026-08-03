#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>

using WorkerFunc = std::function<void(void)>;

class WorkerThreadPrivate;
class WorkerThread : public QThread
{
public:
    WorkerThread(QObject *parent = nullptr);
    ~WorkerThread();

    void setEventLoopEnabled(bool v);

    void runOnWorkerThread(WorkerFunc func, bool sync = false);

    void waitForDone();
    int waitingTasks() const;
    void clearTask();

    void run() override;

private:
    std::shared_ptr<WorkerThreadPrivate> d;
};

#endif // WORKERTHREAD_H
