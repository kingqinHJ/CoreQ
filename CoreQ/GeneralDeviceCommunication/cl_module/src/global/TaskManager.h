#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>

#define s_task TaskManager::instance()

class Task
{
public:
    Task(int priority = 0)
        : priority(priority)
        , state(Suspended)
    {}
    virtual ~Task() {}

    virtual void run() = 0;

    enum State {
        Suspended,      // 挂起状态：所有任务的初始状态都是该状态
        Running,        // 运行状态：任务正在执行
        Stopped,        // 停止状态：任务已经执行结束
    };

    int priority;
    State state;
};

class TaskManagerPrivate;
class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(QObject *parent = nullptr);
    ~TaskManager();

    static TaskManager *instance() { return self; }

    void start();
    void stop();

    int addTask(std::shared_ptr<Task> task);
    int addTask(std::function<void(void)> func);
    void removeTask(int id);

signals:

private:
    static TaskManager *self;
    std::shared_ptr<TaskManagerPrivate> d;
};

#endif // TASKMANAGER_H
