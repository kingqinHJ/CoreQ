#ifndef COMMUNICATIONMANAGER_H
#define COMMUNICATIONMANAGER_H

#include <QObject>
#include "CommunicationInterface.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CommunicationManagerPrivate;
class CLDEVICE_EXPORT CommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationManager(QObject *parent = nullptr);

    CommunicationInterfaceWPtr create(const QString &address, ConnectionType type);
    void destroy(const QString &address, bool immediate = true);
    CommunicationInterfaceWPtr find(const QString &address);

protected:
    void timerEvent(QTimerEvent *e) override;

signals:

private:
    std::shared_ptr<CommunicationManagerPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // COMMUNICATIONMANAGER_H
