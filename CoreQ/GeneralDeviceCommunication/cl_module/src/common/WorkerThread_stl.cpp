#include "WorkerThread.h"

#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

class WorkerThreadPrivate
{
public:
    QString object_name;
    bool abort = false;
    bool running = false;

    struct TaskInfo {
        TaskInfo(WorkerFunc f) : func(f) {}
        WorkerFunc func;
        std::mutex mutex;
        std::condition_variable cond;
    };
    std::queue<std::shared_ptr<TaskInfo>> tasks;
    std::mutex task_mutex;
    std::condition_variable task_empty_cond;

    std::shared_ptr<std::thread> t;
};

WorkerThread::WorkerThread()
{
    d.reset(new WorkerThreadPrivate);
}

WorkerThread::~WorkerThread()
{
    d->abort = true;
    clearTask();
    waitForDone();
}

QString WorkerThread::objectName() const
{
    return d->object_name;
}

void WorkerThread::setObjectName(const QString &n)
{
    d->object_name = n;
}

void WorkerThread::runOnWorkerThread(WorkerFunc func, bool sync)
{
    if (!d->t) {
        d->abort = false;
        d->t.reset(new std::thread(std::bind(&WorkerThread::run, this)));
    }

    if (std::this_thread::get_id() == d->t->get_id()) {
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
    if (!d->t)
        return;

    d->abort = true;
    if (d->t->joinable())
        d->t->join();
    d->t.reset();
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
    d->running = true;
#if defined(Q_OS_LINUX)
    pthread_setname_np(pthread_self(), qUtf8Printable(d->object_name));
#elif defined(Q_OS_WIN)
#endif

    while (!d->abort) {

        std::shared_ptr<WorkerThreadPrivate::TaskInfo> ti;
        {
            std::unique_lock<std::mutex> locker(d->task_mutex);
            if (!d->tasks.empty()) {
                ti = d->tasks.front();
                d->tasks.pop();
            }
            else {
                d->task_empty_cond.wait_for(locker, std::chrono::microseconds(10));
                continue;
            }
        }

        std::unique_lock<std::mutex> locker(ti->mutex);
        if (ti->func)
            ti->func();
        ti->cond.notify_all();
    }
    d->running = false;
}
