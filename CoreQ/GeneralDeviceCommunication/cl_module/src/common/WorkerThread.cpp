#include "WorkerThread.h"

#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

#include <QEventLoop>

class WorkerThreadPrivate
{
public:
    bool abort = false;

    struct TaskInfo {
        TaskInfo(WorkerFunc f) : func(f) {}
        WorkerFunc func;
        std::mutex mutex;
        std::condition_variable cond;
    };
    std::queue<std::shared_ptr<TaskInfo>> tasks;
    std::mutex task_mutex;
    std::condition_variable task_empty_cond;

    bool enable_event_loop = false;
};

WorkerThread::WorkerThread(QObject *parent)
    : QThread(parent)
{
    d.reset(new WorkerThreadPrivate);
}

WorkerThread::~WorkerThread()
{
    d->abort = true;
    clearTask();
    waitForDone();
}

void WorkerThread::setEventLoopEnabled(bool v)
{
    d->enable_event_loop = v;
}

void WorkerThread::runOnWorkerThread(WorkerFunc func, bool sync)
{
    if (!isRunning()) {
        d->abort = false;
        start();
    }

    if (QThread::currentThread() == this) {
        func();
    }
    else {
        std::shared_ptr<WorkerThreadPrivate::TaskInfo> ti(new WorkerThreadPrivate::TaskInfo(func));

        d->task_mutex.lock();
        d->tasks.push(ti);
        d->task_empty_cond.notify_all();
        if (sync) {
            std::unique_lock<std::mutex> locker(ti->mutex);
            d->task_mutex.unlock();
            ti->cond.wait(locker);
        }
        else {
            d->task_mutex.unlock();
        }
    }
}

void WorkerThread::waitForDone()
{
    if (!isRunning())
        return;

    if (QThread::currentThread() == this)
        return;

    d->abort = true;
    while (isRunning()) {
        d->task_empty_cond.notify_all();
        wait(10);
    }
}

int WorkerThread::waitingTasks() const
{
    std::lock_guard<std::mutex> locker(d->task_mutex);
    return d->tasks.size();
}

void WorkerThread::clearTask()
{
    std::lock_guard<std::mutex> locker(d->task_mutex);
    while (!d->tasks.empty()) {
        auto ti = d->tasks.front();
        d->tasks.pop();
        std::lock_guard<std::mutex> locker(ti->mutex);
        ti->cond.notify_all();
    }

    d->task_empty_cond.notify_all();
}

void WorkerThread::run()
{
    QEventLoop event_loop;

    while (!d->abort) {

        if (d->enable_event_loop)
            event_loop.processEvents();

        std::shared_ptr<WorkerThreadPrivate::TaskInfo> ti;
        {
            std::unique_lock<std::mutex> locker(d->task_mutex);
            if (!d->tasks.empty()) {
                ti = d->tasks.front();
                d->tasks.pop();
            }
            else {
                if (d->enable_event_loop)
                    d->task_empty_cond.wait_for(locker, std::chrono::milliseconds(100));
                else
                    d->task_empty_cond.wait(locker);
                continue;
            }
        }

        std::unique_lock<std::mutex> locker(ti->mutex);
        if (ti->func)
            ti->func();
        ti->cond.notify_all();
    }

    if (d->enable_event_loop)
        event_loop.processEvents();
}
