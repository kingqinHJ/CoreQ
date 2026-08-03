#ifndef SYSTEMHELPER_H
#define SYSTEMHELPER_H

#include <QObject>

class SystemHelperPrivate;
class SystemHelper : public QObject
{
    Q_OBJECT
public:
    explicit SystemHelper(QObject *parent = nullptr);
    ~SystemHelper();

    void start();
    void stop();

    float getCpuUsage() const;
    float getCpuUsageSelf() const;

    float getMemUsage() const;
    int64_t getMemTotal() const;
    int64_t getMemUsed() const;
    int64_t getMemUsedSelf() const;

signals:

private:
    std::shared_ptr<SystemHelperPrivate> d;
};

#endif // SYSTEMHELPER_H
