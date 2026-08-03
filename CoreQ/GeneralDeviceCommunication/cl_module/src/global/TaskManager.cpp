#include "TaskManager.h"

#include <QThreadPool>
#include <QThread>
#include <QMap>
#include <QMutex>

class SimpleTask : public Task
{
public:
    SimpleTask(std::function<void(void)> func, int priority = 0)
        : Task(priority)
    {
        m_func = func;
    }

    void run() override {
        if (m_func)
            m_func();
    }

    std::function<void(void)> m_func;
};

class TaskRunnable : public QRunnable
{
public:
    TaskRunnable(int id, std::shared_ptr<Task> task)
        : id(id), task(task)
    {
        setAutoDelete(true);
    }

    void run() override {
        task->state = Task::Running;
        task->run();
        task->state = Task::Stopped;
        task.reset();
        s_task->removeTask(id);
    }

    int id;
    std::shared_ptr<Task> task;
};

class TaskManagerPrivate
{
public:
    QThreadPool thread_poool;

    std::mutex task_mutex;
    QMap<int, TaskRunnable*> task_list;
    int max_task_id = 0;
};

TaskManager *TaskManager::self = nullptr;
TaskManager::TaskManager(QObject *parent) : QObject(parent)
{
    d.reset(new TaskManagerPrivate);
    self = this;
}

TaskManager::~TaskManager()
{
    stop();
    d.reset();
    self = nullptr;
    LOG_THIS();
}

void TaskManager::start()
{
}

void TaskManager::stop()
{
    d->thread_poool.clear();
    d->thread_poool.waitForDone();
}

int TaskManager::addTask(std::shared_ptr<Task> task)
{
    if (!task)
        return -1;

    std::lock_guard<std::mutex> locker(d->task_mutex);
    int id = d->max_task_id++;
    auto runnable = new TaskRunnable(id, task);
    d->task_list[id] = runnable;
    d->thread_poool.start(runnable, task->priority);
    return id;
}

int TaskManager::addTask(std::function<void(void)> func)
{
    std::shared_ptr<Task> task;
    task.reset(new SimpleTask(func));
    return addTask(task);
}

void TaskManager::removeTask(int id)
{
    std::lock_guard<std::mutex> locker(d->task_mutex);
    if (d->task_list.contains(id)) {
        auto runnable = d->task_list[id];
        if (d->thread_poool.tryTake(runnable)) {
            delete runnable;
        }
        d->task_list.remove(id);
    }
}
