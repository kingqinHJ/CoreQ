#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>

using WorkerFunc = std::function<void(void)>;

class WorkerThreadPrivate;
class WorkerThread
{
public:
    WorkerThread();
    ~WorkerThread();

    QString objectName() const;
    void setObjectName(const QString &n);

    void runOnWorkerThread(WorkerFunc func, bool sync = false);

    void waitForDone();

    int waitingTasks() const;

    void clearTask();

    void run();

private:
    std::shared_ptr<WorkerThreadPrivate> d;
};

#endif // WORKERTHREAD_H
