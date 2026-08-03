#ifndef PINGERSYSTEM_H
#define PINGERSYSTEM_H

#include <QObject>

class PingerSystem : public QObject
{
    Q_OBJECT
public:
    explicit PingerSystem(QObject *parent = nullptr);

    bool ping(const QString &host);

private:
};

#endif // PINGERSYSTEM_H
