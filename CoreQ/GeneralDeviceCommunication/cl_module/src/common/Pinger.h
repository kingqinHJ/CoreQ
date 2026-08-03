#ifndef PINGER_H
#define PINGER_H

#include <QString>


class PingerPrivate;
class Pinger
{
public:
    explicit Pinger();
    ~Pinger();

    void abort();
    bool ping(const QString &host);

private:
    std::shared_ptr<PingerPrivate> d;
};

#endif // PINGER_H
