#include "DebounceManager.h"

class DebounceManagerPrivate
{
public:
    void run();
};

void DebounceManagerPrivate::run()
{

}

DebounceManager::DebounceManager(QObject *parent)
    : QObject{parent}
{
    d.reset(new DebounceManagerPrivate);
}

DebounceManager::~DebounceManager()
{

}

void DebounceManager::call(int id, int ms, const std::function<void ()> &func)
{

}
