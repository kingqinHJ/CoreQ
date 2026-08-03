#ifndef DEBOUNCEMANAGER_H
#define DEBOUNCEMANAGER_H

#include <QObject>

class DebounceManagerPrivate;
class DebounceManager : public QObject
{
    Q_OBJECT
public:
    explicit DebounceManager(QObject *parent = nullptr);
    ~DebounceManager();

    void call(int id, int ms, const std::function<void()> &func);

signals:

private:
    std::shared_ptr<DebounceManagerPrivate> d;
};

#endif // DEBOUNCEMANAGER_H
